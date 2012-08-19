#include "ConfigurationImpl.h"
#include "deviceimpl.h"
#include "Interface.h"
#include "Interfaceimpl.h"
#include "usbexception.h"


#include <stdexcept>
#include <algorithm>
#include <xlocale>
#include "wideconvert.h"

LibUSB::ConfigurationImpl::ConfigurationImpl( libusb_config_descriptor* pConfigDescriptor, std::weak_ptr<DeviceImpl> pDeviceImpl)
{

	// Store the config descriptor.
	m_pConfigDescriptor.reset(pConfigDescriptor, ConfigDescriptorDeleter());

	// Store parent device implementation.
	if(pDeviceImpl.expired())
	{
		throw std::runtime_error("ConfigurationImpl constructor has an expired DeviceImpl.");
	}

	m_pDeviceImpl = pDeviceImpl;

}

LibUSB::ConfigurationImpl::~ConfigurationImpl()
{

	// Ensure that all Interface objects have been released/destroyed.

}

std::wstring LibUSB::ConfigurationImpl::DescriptorString( void ) const
{

	if(m_pDeviceImpl.expired())
	{
		throw std::logic_error("LibUSB::ConfigurationImpl::DescriptorString() has an expired DeviceImpl");
	}

	if (m_pConfigDescriptor->iConfiguration == 0)
	{
		// There is no string descriptor
		return L"[No descriptor for this configuration available.]";
	}


	std::wstring resultStr;
	
	try
	{
		resultStr = m_pDeviceImpl.lock()->getStringDescriptorW(m_pConfigDescriptor->iConfiguration);
	}
	catch(LibUSB::LibUSBException &e)
	{
		std::string resultnStr = e.what();
		resultStr = Util::StringToWString(resultnStr);
	}

	return resultStr;

}

uint8_t LibUSB::ConfigurationImpl::getValue() const
{

	return m_pConfigDescriptor->bConfigurationValue;

}

uint8_t LibUSB::ConfigurationImpl::getMaxPower() const
{

	return m_pConfigDescriptor->MaxPower;
	
}

bool LibUSB::ConfigurationImpl::isSelfPowered() const
{
	// Return bit 6 of bmAttributes.
	return (1 == ((m_pConfigDescriptor->bmAttributes & (1 << 6)) >> 6));
}

bool LibUSB::ConfigurationImpl::supportsRemoteWakeup() const
{
	// Return bit 5 of bmAttributes
	return (1 == ((m_pConfigDescriptor->bmAttributes & (1 << 5)) >> 5));
}

bool LibUSB::ConfigurationImpl::hasExtraDescriptors() const
{
	return (m_pConfigDescriptor->extra_length > 0);
}

const unsigned char* LibUSB::ConfigurationImpl::getExtraDescriptors() const
{
	return m_pConfigDescriptor->extra;
}

int LibUSB::ConfigurationImpl::getExtraDescriptorSize() const
{
	return m_pConfigDescriptor->extra_length;
}

void LibUSB::ConfigurationImpl::SetAsActive()
{

	if(m_pDeviceImpl.expired())
	{
		throw std::runtime_error("LibUSB::ConfigurationImpl::SetAsActive() has an expired DeviceImpl");
	}

	m_pDeviceImpl.lock()->setActiveConfiguration(m_pConfigDescriptor->bConfigurationValue);


}

int LibUSB::ConfigurationImpl::NumInterfaces() const
{

	return m_pConfigDescriptor->bNumInterfaces;

}

std::shared_ptr<LibUSB::Interface> LibUSB::ConfigurationImpl::getInterface( int index ) const
{
	/// \note I'm at a loss determining if the (array) index is required to correspond to the interface number.
	
	// Locate the indicated interface
	if (index >= m_pConfigDescriptor->bNumInterfaces)
	{

		/// \note #1 Possibly iterate through the interfaces and check bInterfaceNumber instead???

		throw std::logic_error("LibUSB::ConfigurationImpl::getInterface() - invalid index. (Code Reference \\note #1)");
	}


	// Create the InterfaceImpl object
	const libusb_interface *pInterface = &(m_pConfigDescriptor->interface[index]);
	std::shared_ptr<InterfaceImpl> pInterfaceImpl = std::make_shared<InterfaceImpl>(pInterface, m_pDeviceImpl);
	
	// Create the interface object.
	return std::make_shared<Interface>(pInterfaceImpl);

}

