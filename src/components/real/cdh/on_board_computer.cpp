/*
 * @file on_board_computer.cpp
 * @brief Class to emulate on board computer
 */
#include "on_board_computer.hpp"

OBC::OBC(ClockGenerator* clock_generator) : Component(1, clock_generator) { Initialize(); }

OBC::OBC(int prescaler, ClockGenerator* clock_generator, PowerPort* power_port) : Component(prescaler, clock_generator, power_port) { Initialize(); }

OBC::OBC(int prescaler, ClockGenerator* clock_generator, PowerPort* power_port, const double minimum_voltage_V,
         const double assumed_power_consumption_W)
    : Component(prescaler, clock_generator, power_port) {
  power_port_->SetMinimumVoltage_V(minimum_voltage_V);
  power_port_->SetAssumedPowerConsumption_W(assumed_power_consumption_W);
  Initialize();
}

OBC::~OBC() {}

void OBC::Initialize() {}

void OBC::MainRoutine(const int time_count) { UNUSED(time_count); }

int OBC::ConnectComPort(int port_id, int tx_buffer_size, int rx_buffer_size) {
  if (uart_ports_[port_id] != nullptr) {
    // Port already used
    return -1;
  }
  uart_ports_[port_id] = new UartPort(tx_buffer_size, rx_buffer_size);
  return 0;
}

// Close port and free resources
int OBC::CloseComPort(int port_id) {
  // Port not used
  if (uart_ports_[port_id] == nullptr) return -1;

  UartPort* port = uart_ports_.at(port_id);
  delete port;
  uart_ports_.erase(port_id);
  return 0;
}

int OBC::SendFromObc(int port_id, unsigned char* buffer, int offset, int length) {
  UartPort* port = uart_ports_[port_id];
  if (port == nullptr) return -1;
  return port->WriteTx(buffer, offset, length);
}

int OBC::ReceivedByCompo(int port_id, unsigned char* buffer, int offset, int length) {
  UartPort* port = uart_ports_[port_id];
  if (port == nullptr) return -1;
  return port->ReadTx(buffer, offset, length);
}

int OBC::SendFromCompo(int port_id, unsigned char* buffer, int offset, int length) {
  UartPort* port = uart_ports_[port_id];
  if (port == nullptr) return -1;
  return port->WriteRx(buffer, offset, length);
}

int OBC::ReceivedByObc(int port_id, unsigned char* buffer, int offset, int length) {
  UartPort* port = uart_ports_[port_id];
  if (port == nullptr) return -1;
  return port->ReadRx(buffer, offset, length);
}

int OBC::I2cConnectPort(int port_id, const unsigned char i2c_address) {
  if (i2c_ports_[port_id] != nullptr) {
    // Port already used
  } else {
    i2c_ports_[port_id] = new I2cPort();
  }
  i2c_ports_[port_id]->RegisterDevice(i2c_address);

  return 0;
}

int OBC::I2cCloseComPort(int port_id) {
  // Port not used
  if (i2c_ports_[port_id] == nullptr) return -1;

  I2cPort* port = i2c_ports_.at(port_id);
  delete port;
  i2c_ports_.erase(port_id);
  return 0;
}

int OBC::I2cComponentWriteRegister(int port_id, const unsigned char i2c_address, const unsigned char reg_address, const unsigned char* data,
                                   const unsigned char length) {
  I2cPort* i2c_port = i2c_ports_[port_id];
  for (int i = 0; i < length; i++) {
    i2c_port->WriteRegister(i2c_address, reg_address, data[i]);
  }
  return 0;
}
int OBC::I2cComponentReadRegister(int port_id, const unsigned char i2c_address, const unsigned char reg_address, unsigned char* data,
                                  const unsigned char length) {
  I2cPort* i2c_port = i2c_ports_[port_id];
  for (int i = 0; i < length; i++) {
    data[i] = i2c_port->ReadRegister(reg_address, i2c_address);
  }
  return 0;
}
int OBC::I2cComponentReadCommand(int port_id, const unsigned char i2c_address, unsigned char* data, const unsigned char length) {
  I2cPort* i2c_port = i2c_ports_[port_id];
  i2c_port->ReadCommand(i2c_address, data, length);
  return 0;
}

int OBC::GpioConnectPort(int port_id) {
  if (gpio_ports_[port_id] != nullptr) {
    // Port already used
    return -1;
  }
  gpio_ports_[port_id] = new GpioPort(port_id);
  return 0;
}

int OBC::GpioComponentWrite(int port_id, const bool is_high) {
  GpioPort* port = gpio_ports_[port_id];
  if (port == nullptr) return -1;
  return port->DigitalWrite(is_high);
}

bool OBC::GpioComponentRead(int port_id) {
  GpioPort* port = gpio_ports_[port_id];
  if (port == nullptr) return false;
  return port->DigitalRead();
}
