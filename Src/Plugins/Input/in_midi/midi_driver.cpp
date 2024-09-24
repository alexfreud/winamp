#include "main.h"

MIDI_driver * MIDI_driver::driver_list=0;

MIDI_driver::MIDI_driver()
{
	next=driver_list;
	driver_list=this;
	inited=0;
	device_list=0;
}

void MIDI_driver::reset_devices()
{
	while(device_list)
	{
		MIDI_device * ptr = device_list->next;
		delete device_list;
		device_list = ptr;
	}
}

MIDI_driver::~MIDI_driver()
{
	reset_devices();
}


void MIDI_driver::add_device(MIDI_device * dev)
{
	MIDI_device **ptr = &device_list;
	while(ptr && *ptr) ptr = &(*ptr)->next;
	*ptr=dev;
	dev->next=0;
	dev->driver=this;
}

MIDI_driver * MIDI_driver::driver_enumerate(int n)
{
	MIDI_driver * ptr = driver_list;
	while(ptr && n>0) {ptr=ptr->next;n--;}
	return ptr;
}

int MIDI_driver::driver_count()
{
	int n=0;
	MIDI_driver * ptr = driver_list;
	while(ptr) {ptr=ptr->next;n++;}
	return n;
}
	
MIDI_device * MIDI_driver::device_enumerate(int n)
{
	init();
	MIDI_device * ptr = device_list;
	while(ptr && n>0) {ptr=ptr->next;n--;}
	return ptr;
}

int MIDI_driver::device_count()
{
	init();
	int n=0;
	MIDI_device * ptr = device_list;
	while(ptr) {ptr=ptr->next;n++;}
	return n;
}

MIDI_device * MIDI_driver::find_device(GUID guid_driver,GUID guid_device)
{
	MIDI_driver * driver_ptr = find_driver(guid_driver);
	if (!driver_ptr) return 0;
	MIDI_device * device_ptr;
	int idx=0;
	while(device_ptr = driver_ptr->device_enumerate(idx++))
	{
		if (device_ptr->get_guid()==guid_device) return device_ptr;
	}
	return 0;
}

MIDI_driver * MIDI_driver::find_driver(GUID guid_driver)
{
	MIDI_driver * driver_ptr = driver_list;
	while(driver_ptr)
	{
		if (driver_ptr->get_guid()==guid_driver) break;
		driver_ptr = driver_ptr->next;
	}
	return driver_ptr;
}

MIDI_device * MIDI_driver::find_device_default()
{
	MIDI_driver * driver_ptr = driver_list;
	while(driver_ptr)
	{
		if (driver_ptr->is_default())
		{
			MIDI_device * device_ptr;
			int idx=0;
			while(device_ptr = driver_ptr->device_enumerate(idx++))
			{
				if (device_ptr->is_default()) return device_ptr;
			}
		}
		driver_ptr = driver_ptr->next;
	}
	return 0;
}

void MIDI_driver::shutdown()
{
	MIDI_driver * driver_ptr = driver_list;
	while(driver_ptr)
	{
		driver_ptr->deinit();
		driver_ptr = driver_ptr->next;
	}
}