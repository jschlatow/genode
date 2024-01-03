#include <util/formatted_output.h>

extern "C" {
#include "acresrc.h"
}


namespace Btacpi {

typedef String<128> Pathname;
typedef String<64>  Hid;
typedef String<64>  Cid;

struct Device
{
	ACPI_HANDLE       handle;
	ACPI_DEVICE_INFO *info;

	Device(ACPI_HANDLE handle) : handle(handle)
	{
		AcpiGetObjectInfo(handle, &info);
	}

	~Device() { ACPI_FREE(info); }

	Pathname pathname() const
	{
		Acpica::Buffer<char [128]> path;
		AcpiGetName(handle, ACPI_FULL_PATHNAME_NO_TRAILING, &path);

		return (char const *)path.Pointer;
	}

	Hid hid() const
	{
		if (!(info->Valid & ACPI_VALID_HID))
			return "";

//		if (info->HardwareId.Length > 8)
//			log("--------------- ", info->HardwareId.Length, "'", (char const *)HardwareId.String);

		return (char const *)info->HardwareId.String;
	}

	Cid cid() const
	{
		if (!(info->Valid & ACPI_VALID_CID))
			return "";

		/* only first compatible ID returned */
		return (char const *)info->CompatibleIdList.Ids[0].String;
	}

	ACPI_HANDLE parent() const
	{
		ACPI_HANDLE parent;
		AcpiGetParent(handle, &parent);

		return parent;
	}

	bool present_ok()
	{
		Acpica::Buffer<ACPI_OBJECT> retval;
		ACPI_STATUS ret =
			AcpiEvaluateObjectTyped(handle, ACPI_STRING("_STA"), nullptr,
			                        &retval, ACPI_TYPE_INTEGER);
		switch (ret) {
		case AE_NOT_FOUND:
			return true;

		case AE_OK: {
				unsigned const sta = retval.object.Integer.Value;
				return sta & ACPI_STA_DEVICE_PRESENT
				    && sta & ACPI_STA_DEVICE_FUNCTIONING;
			} break;

		default:
			warning("AcpiEvaluateObjectTyped returned ", AcpiFormatException(ret));
		}

		return false;
	}
};


ACPI_STATUS detect_resources(ACPI_RESOURCE *resource, void *context)
{
	Device const &dev = *(Device const *)context;

	switch (resource->Type) {
	case ACPI_RESOURCE_TYPE_IRQ:
		log(" ", Right_aligned(8, dev.hid()),
		    " ", Right_aligned(8, dev.cid()),
		    " IRQ",
		    " n=", resource->Data.Irq.InterruptCount,
		    " first=", resource->Data.Irq.Interrupts[0]);
		break;

	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
		log(" ", Right_aligned(8, dev.hid()),
		    " ", Right_aligned(8, dev.cid()),
		    " EXTIRQ",
		    " n=", resource->Data.ExtendedIrq.InterruptCount,
		    " first=", resource->Data.ExtendedIrq.Interrupts[0]);
		break;

	case ACPI_RESOURCE_TYPE_SERIAL_BUS:
		switch (resource->Data.CommonSerialBus.Type) {
		case ACPI_RESOURCE_SERIAL_TYPE_I2C:
			log(" ", Right_aligned(8, dev.hid()),
			    " ", Right_aligned(8, dev.cid()),
			    " I2C",
			    " (", resource->Length, ")",
			    " type=", resource->Data.CommonSerialBus.Type,
			    " length=", resource->Data.CommonSerialBus.TypeDataLength,
			    " vlength=", resource->Data.CommonSerialBus.VendorLength,
			    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
			break;
		case ACPI_RESOURCE_SERIAL_TYPE_SPI:
			log(" ", Right_aligned(8, dev.hid()),
			    " ", Right_aligned(8, dev.cid()),
			    " SPI",
			    " (", resource->Length, ")",
			    " type=", resource->Data.CommonSerialBus.Type,
			    " length=", resource->Data.CommonSerialBus.TypeDataLength,
			    " vlength=", resource->Data.CommonSerialBus.VendorLength,
			    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
			break;
		case ACPI_RESOURCE_SERIAL_TYPE_UART:
			log(" ", Right_aligned(8, dev.hid()),
			    " ", Right_aligned(8, dev.cid()),
			    " UART",
			    " (", resource->Length, ")",
			    " type=", resource->Data.CommonSerialBus.Type,
			    " length=", resource->Data.CommonSerialBus.TypeDataLength,
			    " vlength=", resource->Data.CommonSerialBus.VendorLength,
			    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
			break;
		} break;

	case ACPI_RESOURCE_TYPE_GPIO:
		log(" ", Right_aligned(8, dev.hid()),
		    " ", Right_aligned(8, dev.cid()),
		    " GPIO",
		    " type=", resource->Data.Gpio.ConnectionType ? "IO" : "INT",
		    " pintablelength=", resource->Data.Gpio.PinTableLength,
		    " pintable[0]=", resource->Data.Gpio.PinTable[0]);
//		    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
		break;

	default:
		log(" ", Right_aligned(8, dev.hid()),
		    " ", Right_aligned(8, dev.cid()),
		    " RESOURCE TYPE ", resource->Type, " not handled");
		break;

	case ACPI_RESOURCE_TYPE_END_TAG: /* ignore */ break;
	}

	return AE_OK;
}


ACPI_STATUS display_devices(ACPI_HANDLE handle, UINT32 level,
                            void * /* context */, void ** /* retval */)
{
	Device dev { handle };

	if (dev.present_ok()) {
		warning(dev.pathname());
		AcpiWalkResources(handle, ACPI_STRING("_CRS"), detect_resources, &dev);
	}

	return AE_OK;
}

ACPI_STATUS display_methods(ACPI_HANDLE handle, UINT32 level,
                            void * /* context */, void ** /* retval */)
{
//	warning(__func__, ":", __LINE__, " handle=", handle, " level=", level);

	Device method { handle };
	Device parent { method.parent() };

	warning(__func__, ":", __LINE__,
	        " m=", method.pathname(),
	        " p=", parent.pathname(),
	        " h=", parent.hid(), " c=", parent.cid());

	return AE_OK;
}

void another_test()
{
//	AcpiDbgLevel |= ACPI_LV_INFO;
//	AcpiDbgLayer |= ACPI_TABLES;

	if (false) {
		Acpica::Buffer<char [0x1000]> resources;
		AcpiGetCurrentResources(ACPI_ROOT_OBJECT, &resources);
		AcpiRsDumpResourceList(ACPI_CAST_PTR(ACPI_RESOURCE, resources.Pointer));
	} else {
		log("-- ACPI_TYPE_DEVICE --");
		AcpiWalkNamespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
		                  display_devices, nullptr, nullptr, nullptr);
		log("-- ACPI_TYPE_THERMAL --");
		AcpiWalkNamespace(ACPI_TYPE_THERMAL, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
		                  display_devices, nullptr, nullptr, nullptr);
		log("-- ACPI_TYPE_PROCESSOR --");
		AcpiWalkNamespace(ACPI_TYPE_PROCESSOR, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
		                  display_devices, nullptr, nullptr, nullptr);
	}

//	AcpiWalkNamespace(ACPI_TYPE_METHOD, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
//	                  display_methods, nullptr, nullptr, nullptr);
}

} /* namespace Btacpi */
