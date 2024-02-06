/**
 * @file gnss_satellites.cpp
 * @brief Class to calculate GNSS satellite position and clock
 */

#include "gnss_satellites.hpp"

#include <library/gnss/igs_product_name_handling.hpp>
#include <library/gnss/sp3_file_reader.hpp>

#include "environment/global/physical_constants.hpp"
#include "library/initialize/initialize_file_access.hpp"
#include "library/logger/log_utility.hpp"
#include "library/math/constants.hpp"
#include "library/time_system/date_time_format.hpp"
#include "library/utilities/macros.hpp"

const size_t kNumberOfInterpolation = 9;

void GnssSatellites::Initialize(const std::vector<Sp3FileReader>& sp3_files, const EpochTime start_time) {
  sp3_files_ = sp3_files;
  current_epoch_time_ = start_time;

  // Get the initialize SP3 file
  Sp3FileReader initial_sp3_file = sp3_files_[0];
  if (!GetCurrentSp3File(initial_sp3_file, start_time)) {
    std::cout << "[Error] GNSS satellites: Calculation time mismatch with SP3 files." << std::endl;
    return;
  }

  // Get general info
  number_of_calculated_gnss_satellites_ = initial_sp3_file.GetNumberOfSatellites();
  const size_t nearest_epoch_id = initial_sp3_file.SearchNearestEpochId(start_time);
  const size_t half_interpolation_number = kNumberOfInterpolation / 2;
  if (nearest_epoch_id >= half_interpolation_number) {
    reference_interpolation_id_ = nearest_epoch_id - half_interpolation_number;
  }
  reference_time_ = EpochTime(initial_sp3_file.GetEpochData(reference_interpolation_id_));

  // Initialize orbit
  orbit_.assign(number_of_calculated_gnss_satellites_, InterpolationOrbit(kNumberOfInterpolation));

  // Initialize clock
  std::vector<double> temp;
  temp.assign(kNumberOfInterpolation, -1.0);
  clock_.assign(number_of_calculated_gnss_satellites_, libra::Interpolation(temp, temp));

  // Initialize
  for (size_t i = 0; i < kNumberOfInterpolation; i++) {
    for (size_t gnss_idx = 0; gnss_idx < number_of_calculated_gnss_satellites_; gnss_idx++) {
      EpochTime time_at_epoch_id = EpochTime(initial_sp3_file.GetEpochData(reference_interpolation_id_));
      double time_diff_s = time_at_epoch_id.GetTimeWithFraction_s() - reference_time_.GetTimeWithFraction_s();
      libra::Vector<3> sp3_position_m = 1000.0 * initial_sp3_file.GetSatellitePosition_km(reference_interpolation_id_, gnss_idx);

      orbit_[gnss_idx].PushAndPopData(time_diff_s, sp3_position_m);
      clock_[gnss_idx].PushAndPopData(time_diff_s, initial_sp3_file.GetSatelliteClockOffset(reference_interpolation_id_, gnss_idx));
    }
    reference_interpolation_id_++;
    // File update
    if (reference_interpolation_id_ >= initial_sp3_file.GetNumberOfEpoch()) {
      reference_interpolation_id_ = 0;
      sp3_file_id_++;
      if (sp3_file_id_ >= sp3_files_.size()) {
        std::cout << "[Error] GNSS satellites: SP3 file range over." << std::endl;
      }
      initial_sp3_file = sp3_files_[sp3_file_id_];
    }
  }

  return;
}

void GnssSatellites::Update(const SimulationTime& simulation_time) {
  if (!IsCalcEnabled()) return;

  // Get time
  UTC current_utc = simulation_time.GetCurrentUtc();
  DateTime current_date_time((size_t)current_utc.year, (size_t)current_utc.month, (size_t)current_utc.day, (size_t)current_utc.hour,
                             (size_t)current_utc.minute, current_utc.second);
  current_epoch_time_ = EpochTime(current_date_time);

  // TODO Check file updates

  // SP3 file
  Sp3FileReader sp3_file = sp3_files_[sp3_file_id_];

  double diff_s = current_epoch_time_.GetTimeWithFraction_s() - reference_time_.GetTimeWithFraction_s();
  double medium_time_s = orbit_[0].GetTimeList()[4];
  if (diff_s > medium_time_s) {
    for (size_t gps_idx = 0; gps_idx < 32; gps_idx++) {
      EpochTime sp3_time = EpochTime(sp3_file.GetEpochData(reference_interpolation_id_));
      double time_diff_s = sp3_time.GetTimeWithFraction_s() - reference_time_.GetTimeWithFraction_s();
      libra::Vector<3> sp3_position_m = 1000.0 * sp3_file.GetSatellitePosition_km(reference_interpolation_id_, gps_idx);

      orbit_[gps_idx].PushAndPopData(time_diff_s, sp3_position_m);
      clock_[gps_idx].PushAndPopData(time_diff_s, sp3_file.GetSatelliteClockOffset(reference_interpolation_id_, gps_idx));
    }
    reference_interpolation_id_++;
    // File update
    if (reference_interpolation_id_ >= sp3_file.GetNumberOfEpoch()) {
      reference_interpolation_id_ = 0;
      sp3_file_id_++;
      if (sp3_file_id_ >= sp3_files_.size()) {
        std::cout << "[Error] GNSS satellites: SP3 file range over." << std::endl;
      }
      sp3_file = sp3_files_[sp3_file_id_];
    }
  }

  return;
}

libra::Vector<3> GnssSatellites::GetPosition_ecef_m(const size_t gnss_satellite_id, const EpochTime time) const {
  if (gnss_satellite_id > number_of_calculated_gnss_satellites_) return libra::Vector<3>(0.0);

  EpochTime target_time;

  if (time.GetTime_s() == 0) {
    target_time = current_epoch_time_;
  } else {
    target_time = time;
  }

  double diff_s = target_time.GetTimeWithFraction_s() - reference_time_.GetTimeWithFraction_s();
  if (diff_s < 0.0 || diff_s > 1e6) return libra::Vector<3>(0.0);

  const double kOrbitalPeriodCorrection_s = 24 * 60 * 60 * 1.003;  // See http://acc.igs.org/orbits/orbit-interp_gpssoln03.pdf
  return orbit_[gnss_satellite_id].CalcPositionWithTrigonometric(diff_s, libra::tau / kOrbitalPeriodCorrection_s);
}

double GnssSatellites::GetClock_s(const size_t gnss_satellite_id, const EpochTime time) const {
  if (gnss_satellite_id > number_of_calculated_gnss_satellites_) return 0.0;

  EpochTime target_time;

  if (time.GetTime_s() == 0) {
    target_time = current_epoch_time_;
  } else {
    target_time = time;
  }

  double diff_s = target_time.GetTimeWithFraction_s() - reference_time_.GetTimeWithFraction_s();
  if (diff_s < 0.0 || diff_s > 1e6) return 0.0;

  return clock_[gnss_satellite_id].CalcPolynomial(diff_s) * 1e-6;
}

bool GnssSatellites::GetCurrentSp3File(Sp3FileReader& current_sp3_file, const EpochTime current_time) {
  for (size_t i = 0; i < sp3_files_.size(); i++) {
    EpochTime sp3_start_time(sp3_files_[i].GetStartEpochDateTime());
    double diff_s = current_time.GetTimeWithFraction_s() - sp3_start_time.GetTimeWithFraction_s();
    if (diff_s < 0.0) {
      // Error
      return false;
    } else if (diff_s < 24 * 60 * 60) {
      current_sp3_file = sp3_files_[i];
      sp3_file_id_ = i;
      return true;
    }
  }
  return false;
}

std::string GnssSatellites::GetLogHeader() const {
  std::string str_tmp = "";

  // TODO: Add log output for other navigation systems
  for (size_t gps_index = 0; gps_index < kNumberOfGpsSatellite; gps_index++) {
    str_tmp += WriteVector("GPS" + std::to_string(gps_index) + "_position", "ecef", "m", 3);
    str_tmp += WriteScalar("GPS" + std::to_string(gps_index) + "_clock_offset", "s");
  }

  return str_tmp;
}

std::string GnssSatellites::GetLogValue() const {
  std::string str_tmp = "";

  for (size_t gps_index = 0; gps_index < kNumberOfGpsSatellite; gps_index++) {
    str_tmp += WriteVector(GetPosition_ecef_m(gps_index), 16);
    str_tmp += WriteScalar(GetClock_s(gps_index));
  }

  return str_tmp;
}

GnssSatellites* InitGnssSatellites(const std::string file_name, const EarthRotation& earth_rotation, const SimulationTime& simulation_time) {
  IniAccess ini_file(file_name);
  char section[] = "GNSS_SATELLITES";

  const bool is_calc_enable = ini_file.ReadEnable(section, INI_CALC_LABEL);
  const bool is_log_enable = ini_file.ReadEnable(section, INI_LOG_LABEL);

  GnssSatellites* gnss_satellites = new GnssSatellites(earth_rotation, is_calc_enable, is_log_enable);
  if (!gnss_satellites->IsCalcEnabled()) {
    return gnss_satellites;
  }

  const std::string directory_path = ini_file.ReadString(section, "directory_path");
  const std::string file_name_header = ini_file.ReadString(section, "file_name_header");
  const std::string orbit_data_period = ini_file.ReadString(section, "orbit_data_period");
  const std::string clock_file_name_footer = ini_file.ReadString(section, "clock_file_name_footer");
  bool use_sp3_for_clock = false;
  if (clock_file_name_footer == (orbit_data_period + "_ORB.SP3")) {
    use_sp3_for_clock = true;
  }

  // Duration
  const size_t start_date = (size_t)ini_file.ReadInt(section, "start_date");
  const size_t end_date = (size_t)ini_file.ReadInt(section, "end_date");
  if (start_date > end_date) {
    std::cout << "[ERROR] GNSS satellite initialize: start_date is larger than the end date." << std::endl;
  }

  // Read all product files
  std::vector<Sp3FileReader> sp3_file_readers;

  size_t read_file_date = start_date;
  while (read_file_date <= end_date) {
    std::string sp3_file_name = GetOrbitClockFinalFileName(file_name_header, read_file_date, orbit_data_period);
    std::string sp3_full_file_path = directory_path + sp3_file_name;

    // Read SP3
    sp3_file_readers.push_back(Sp3FileReader(sp3_full_file_path));

    // Clock file
    if (!use_sp3_for_clock) {
      std::string clk_file_name =
          GetOrbitClockFinalFileName(file_name_header, read_file_date, clock_file_name_footer.substr(0, 3), clock_file_name_footer.substr(4, 7));
      std::string clk_full_file_path = directory_path + clk_file_name;
      // TODO: Read CLK file
    }

    // Increment
    read_file_date = IncrementYearDoy(read_file_date);
  }

  //
  DateTime start_date_time((size_t)simulation_time.GetStartYear(), (size_t)simulation_time.GetStartMonth(), (size_t)simulation_time.GetStartDay(),
                           (size_t)simulation_time.GetStartHour(), (size_t)simulation_time.GetStartMinute(), simulation_time.GetStartSecond());
  EpochTime start_epoch_time(start_date_time);
  gnss_satellites->Initialize(sp3_file_readers, start_epoch_time);

  return gnss_satellites;
}
