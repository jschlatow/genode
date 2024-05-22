#include <util/formatted_output.h>

extern "C" {
#include "acresrc.h"
}


namespace Btacpi {

typedef String<8>   Name;
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

	Name name() const
	{
		Acpica::Buffer<char [Name::capacity()]> name;
		AcpiGetName(handle, ACPI_SINGLE_NAME, &name);

		return { (char const *)name.Pointer };
	}

	Pathname pathname() const
	{
		Acpica::Buffer<char [Pathname::capacity()]> path;
		AcpiGetName(handle, ACPI_FULL_PATHNAME_NO_TRAILING, &path);

		return { (char const *)path.Pointer };
	}

	Hid hid() const
	{
		if (!(info->Valid & ACPI_VALID_HID))
			return "";

//		if (info->HardwareId.Length > 8)
//			log("--------------- ", info->HardwareId.Length, "'", (char const *)HardwareId.String);

		return { (char const *)info->HardwareId.String };
	}

	Cid cid() const
	{
		if (!(info->Valid & ACPI_VALID_CID))
			return "";

		/* only first compatible ID returned */
		return { (char const *)info->CompatibleIdList.Ids[0].String };
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
	struct Io;
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


struct Resource::Io : ACPI_RESOURCE_IO
{
	Io(ACPI_RESOURCE_IO const &res) : ACPI_RESOURCE_IO(res) { }

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, "IO");
		print(out, " ", Hex(IoDecode));
		print(out, " ", Hex(Alignment));
		print(out, " ", AddressLength);
		print(out, " ", Hex(Minimum));
		print(out, " ", Hex(Maximum));
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
	case ACPI_RESOURCE_TYPE_IO:           print(out, Io(Data.Io)); break;
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


ACPI_STATUS detect_resource(ACPI_RESOURCE *resource, void *context)
{
	if (resource->Type == ACPI_RESOURCE_TYPE_END_TAG)
		return AE_OK;

	Device const &dev = *(Device const *)context;

	log(" ", Right_aligned(8, dev.hid()),
	    " ", Right_aligned(8, dev.cid()),
	    " ", Resource(*resource));

	return AE_OK;
}


ACPI_STATUS display_method(ACPI_HANDLE handle, UINT32 level,
                            void * /* context */, void ** /* retval */)
{
//	warning(__func__, ":", __LINE__, " handle=", handle, " level=", level);

	Device method { handle };
	Device parent { method.parent() };

	log(" ", method.name(), " - ", method.pathname(), " ", handle);

	return AE_OK;
}


ACPI_STATUS display_device(ACPI_HANDLE handle, UINT32 level,
                           void * /* context */, void ** /* retval */)
{
	Device dev { handle };

	if (dev.present_ok()) {
		warning(dev.pathname());
		AcpiWalkResources(handle, ACPI_STRING("_CRS"), detect_resource, &dev);

		ACPI_HANDLE child = NULL;
		while (true) {
			if (AcpiGetNextObject(ACPI_TYPE_METHOD, dev.handle, child, &child) != AE_OK)
				break;
			display_method(child, 0, 0, 0);
		}

		if (false) {
			child = NULL;
			while (true) {
				if (AcpiGetNextObject(ACPI_TYPE_ANY, dev.handle, child, &child) != AE_OK)
					break;
				Device x { child };
				log("--- ", x.name(), " - ", x.pathname());
			}
		}

		/* TODO call HID _DSM with func=0 to query avail funcs */
		if (dev.cid() == "PNP0C50") {
			ACPI_OBJECT                 args[4];
			ACPI_OBJECT_LIST            in { 4, args };
			Acpica::Buffer<ACPI_OBJECT> out;

			ACPI_UUID guid { ACPI_INIT_UUID(0x3cdff6f7, 0x4267, 0x4555, 0xad, 0x05, 0xb3, 0x0a, 0x3d, 0x89, 0x38, 0xde) };

			args[0].Type = ACPI_TYPE_BUFFER;
			args[0].Buffer.Length = sizeof(guid);
			args[0].Buffer.Pointer = guid.Data;
			args[1].Type = ACPI_TYPE_INTEGER;
			args[1].Integer.Value = 1; /* revision */
			args[2].Type = ACPI_TYPE_INTEGER;
			args[2].Integer.Value = 1; /* function (0: query 1: HID descriptor address (2 bytes) */
			args[3].Type = ACPI_TYPE_PACKAGE;
			args[3].Package.Count = 0;
			args[3].Package.Elements = nullptr;

			ACPI_STATUS ret = AcpiEvaluateObjectTyped(dev.handle, ACPI_STRING("_DSM"), &in, &out, ACPI_TYPE_INTEGER);

			log("++++ ret=", ret, " type=", (ACPI_OBJECT_TYPE)out.object.Type, " result=", (UINT64)out.object.Integer.Value);
		}
	}

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
		                  display_device, nullptr, nullptr, nullptr);
		log("-- ACPI_TYPE_THERMAL --");
		AcpiWalkNamespace(ACPI_TYPE_THERMAL, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
		                  display_device, nullptr, nullptr, nullptr);
		log("-- ACPI_TYPE_PROCESSOR --");
		AcpiWalkNamespace(ACPI_TYPE_PROCESSOR, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
		                  display_device, nullptr, nullptr, nullptr);
	}

//	AcpiWalkNamespace(ACPI_TYPE_METHOD, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
//	                  display_method, nullptr, nullptr, nullptr);
}

} /* namespace Btacpi */
