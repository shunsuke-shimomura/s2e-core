#include "I2CPort.h"

I2CPort::I2CPort(void)
{
}

I2CPort::I2CPort(const unsigned char max_register_number)
:max_register_number_(max_register_number)
{
}

void I2CPort::RegisterDevice(const unsigned char i2c_addr)
{
  for (unsigned char i=0;i<max_register_number_;i++)
  {
    device_registers_[std::make_pair(i2c_addr, i)] = 0x00;
  }
}

int I2CPort::WriteRegister(const unsigned char i2c_addr, const unsigned char reg_addr)
{
  if(reg_addr >= max_register_number_) return 0;
  saved_reg_addr_ = reg_addr;
  return 1;
}

int I2CPort::WriteRegister(const unsigned char i2c_addr, const unsigned char reg_addr, const unsigned char value)
{
  if(reg_addr >= max_register_number_) return 0;
  saved_reg_addr_ = reg_addr;
  device_registers_[std::make_pair(i2c_addr, reg_addr)] = value;
  return 1;
}

/*
int I2CPort::WriteRegister(const unsigned char i2c_addr, const unsigned char reg_addr, float value)
{
  if(reg_addr >= max_register_number_) return 0;
  saved_reg_addr_ = reg_addr;
  unsigned char* value_ptr = reinterpret_cast<unsigned char*>(&value);
  for(size_t i = 0; i < sizeof(float); i++)
  {
    WriteRegister(i2c_addr,reg_addr, value_ptr[i]);
  }
  return 1;
}
*/

unsigned char I2CPort::ReadRegister(const unsigned char i2c_addr)
{
  unsigned char ret = device_registers_[std::make_pair(i2c_addr, saved_reg_addr_)];
  saved_reg_addr_++;
  if(saved_reg_addr_ >= max_register_number_) saved_reg_addr_ = 0;
  return ret;
}

unsigned char I2CPort::ReadRegister(const unsigned char i2c_addr, const unsigned char reg_addr)
{
  if(reg_addr >= max_register_number_) return 0;
  saved_reg_addr_ = reg_addr;
  unsigned char ret = device_registers_[std::make_pair(i2c_addr, saved_reg_addr_)];
  return ret;
}