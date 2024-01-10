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


struct Resource : ACPI_RESOURCE
{
	struct Irq;
	struct Extended_irq;
	struct Gpio;

	static char const * producer_consumer(UINT8 v)
	{
		switch (v) {
		case ACPI_PRODUCER: return "producer";
		case ACPI_CONSUMER: return "consumer";
		default: return "?";
		}
	}

	static char const * triggering(UINT8 v)
	{
		switch (v) {
		case ACPI_LEVEL_SENSITIVE: return "level";
		case ACPI_EDGE_SENSITIVE:  return "edge";
		default: return "?";
		}
	}

	static char const * polarity(UINT8 v)
	{
		switch (v) {
		case ACPI_ACTIVE_HIGH: return "high";
		case ACPI_ACTIVE_LOW:  return "low";
		case ACPI_ACTIVE_BOTH: return "both";
		default: return "?";
		}
	}

	static char const * sharable(UINT8 v)
	{
		switch (v) {
		case ACPI_EXCLUSIVE: return "exclusive";
		case ACPI_SHARED:    return "shared";
		default: return "?";
		}
	}

	/*
	 * FIXME
	 *
	 * - Length is dynamic and may include lists
	 * - better use AcpiGetCurrentResources() into buffer owned by Device
	 * - parse via AcpiWalkResourceBuffer()
	 * - then all references from Resource and sub classes share a life-time
	 *   with Device
	 */
	Resource(ACPI_RESOURCE const &res) : ACPI_RESOURCE(res) { }

	void print(Genode::Output &out) const;
};


struct Resource::Irq : ACPI_RESOURCE_IRQ
{
	Irq(ACPI_RESOURCE_IRQ const &res) : ACPI_RESOURCE_IRQ(res)
	{
		if (InterruptCount > 1)
			warning("Resource::Irq: only first of ", InterruptCount, " IRQs supported");
	}

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, "IRQ");
		print(out, " ", Interrupts[0]);
		print(out, " ", triggering(Triggering));
		print(out, " ", polarity(Polarity));
		print(out, " ", sharable(Sharable));
	}
};


struct Resource::Extended_irq : ACPI_RESOURCE_EXTENDED_IRQ
{
	Extended_irq(ACPI_RESOURCE_EXTENDED_IRQ const &res) : ACPI_RESOURCE_EXTENDED_IRQ(res)
	{
		if (InterruptCount > 1)
			warning("Resource::Extended_irq: only first of ", InterruptCount, " IRQs supported");
	}

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, "EXTIRQ");
		print(out, " ", Interrupts[0]);
		print(out, " ", triggering(Triggering));
		print(out, " ", polarity(Polarity));
		print(out, " ", sharable(Sharable));
		print(out, " ", producer_consumer(ProducerConsumer));

		/* FIXME StringPtr is outside ACPI_RESOURCE_EXTENDED_IRQ */
		if (ResourceSource.StringPtr)
			print(out, " ", (char const *)ResourceSource.StringPtr);
	}
};


struct Resource::Gpio : ACPI_RESOURCE_GPIO
{
	Gpio(ACPI_RESOURCE_GPIO const &res) : ACPI_RESOURCE_GPIO(res)
	{
		/* *PinTable of PinTableLength */
		/* *VendorData of VendorLength */
	}

	static char const * connection_type(UINT8 v)
	{
		switch (v) {
		case ACPI_RESOURCE_GPIO_TYPE_INT: return "int";
		case ACPI_RESOURCE_GPIO_TYPE_IO:  return "io";
		default: return "?";
		}
	}

	static char const * pin_config(UINT8 v)
	{
		switch (v) {
		case ACPI_PIN_CONFIG_DEFAULT:  return "default";
		case ACPI_PIN_CONFIG_PULLUP:   return "pullup";
		case ACPI_PIN_CONFIG_PULLDOWN: return "pulldown";
		case ACPI_PIN_CONFIG_NOPULL:   return "nopull";
		default: return "?";
		}
	}

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, "GPIO");
		print(out, " ", connection_type(ConnectionType));
		print(out, " ", pin_config(PinConfig));
		print(out, " ", triggering(Triggering));
		print(out, " ", polarity(Polarity));
		print(out, " ", sharable(Sharable));
		print(out, " ", producer_consumer(ProducerConsumer));
	}
};


void Resource::print(Genode::Output &out) const
{
	using Genode::print;

	switch (Type) {
	case ACPI_RESOURCE_TYPE_IRQ:          print(out, Irq(Data.Irq)); break;
	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ: print(out, Extended_irq(Data.ExtendedIrq)); break;

//	case ACPI_RESOURCE_TYPE_SERIAL_BUS:
//		switch (resource->Data.CommonSerialBus.Type) {
//		case ACPI_RESOURCE_SERIAL_TYPE_I2C:
//			log(" ", Right_aligned(8, dev.hid()),
//			    " ", Right_aligned(8, dev.cid()),
//			    " I2C",
//			    " (", resource->Length, ")",
//			    " type=", resource->Data.CommonSerialBus.Type,
//			    " length=", resource->Data.CommonSerialBus.TypeDataLength,
//			    " vlength=", resource->Data.CommonSerialBus.VendorLength,
//			    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
//			break;
//		case ACPI_RESOURCE_SERIAL_TYPE_SPI:
//			log(" ", Right_aligned(8, dev.hid()),
//			    " ", Right_aligned(8, dev.cid()),
//			    " SPI",
//			    " (", resource->Length, ")",
//			    " type=", resource->Data.CommonSerialBus.Type,
//			    " length=", resource->Data.CommonSerialBus.TypeDataLength,
//			    " vlength=", resource->Data.CommonSerialBus.VendorLength,
//			    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
//			break;
//		case ACPI_RESOURCE_SERIAL_TYPE_UART:
//			log(" ", Right_aligned(8, dev.hid()),
//			    " ", Right_aligned(8, dev.cid()),
//			    " UART",
//			    " (", resource->Length, ")",
//			    " type=", resource->Data.CommonSerialBus.Type,
//			    " length=", resource->Data.CommonSerialBus.TypeDataLength,
//			    " vlength=", resource->Data.CommonSerialBus.VendorLength,
//			    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
//			break;
//		} break;
//
//	case ACPI_RESOURCE_TYPE_GPIO:
//		log(" ", Right_aligned(8, dev.hid()),
//		    " ", Right_aligned(8, dev.cid()),
//		    " GPIO",
//		    " type=", resource->Data.Gpio.ConnectionType ? "IO" : "INT",
//		    " pintablelength=", resource->Data.Gpio.PinTableLength,
//		    " pintable[0]=", resource->Data.Gpio.PinTable[0]);
////		    " ", (char const *)resource->Data.CommonSerialBus.ResourceSource.StringPtr);
//		break;

	default:
		print(out, "unknown type ", Type);
		break;
	}
}


ACPI_STATUS detect_resources(ACPI_RESOURCE *resource, void *context)
{
	if (resource->Type == ACPI_RESOURCE_TYPE_END_TAG)
		return AE_OK;

	Device const &dev = *(Device const *)context;

	log(" ", Right_aligned(8, dev.hid()),
	    " ", Right_aligned(8, dev.cid()),
	    " ", Resource(*resource));

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
