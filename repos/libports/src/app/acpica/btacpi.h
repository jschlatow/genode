/*
 * TBD
 *
 * [acpi]   Advanced Configuration and Power Interface (ACPI) Specification
 *          Release 6.5, August 29, 2022
 *          https://uefi.org/specs/ACPI/6.5/
 *
 * [acpica] ACPI Component Architecture User Guide and Programmer Reference
 *          Revision 6.3, September 14, 2021
 *          https://github.com/acpica/acpica/blob/1ceb58cdec7f08dfccba36652193d4d20e0a1063/documents/acpica-reference.pdf
 */
#include <util/formatted_output.h>

extern "C" {
#include "acresrc.h"
}


namespace Btacpi {

using Name     = String<8>;
using Pathname = String<128>;
using Id       = String<64>;
using Hid      = Id;
using Cid      = Id;

struct Buffer : ACPI_BUFFER
{
	Buffer() : ACPI_BUFFER(ACPI_ALLOCATE_BUFFER, nullptr) { }

	~Buffer()
	{
		/*
		 * [acpica] 8.10 Memory Management
		 *
		 * if * ACPI_ALLOCATE_BUFFER is used [...] AcpiOsFree should be used to
		 * free the buffer.
		 */
		if (Pointer) AcpiOsFree(Pointer);
	}

	auto with_object(auto const &fn) { return fn(*(ACPI_OBJECT *)Pointer); }
};


struct Device
{
	ACPI_HANDLE       handle;
	ACPI_DEVICE_INFO *info;

	Device(ACPI_HANDLE handle) : handle(handle)
	{
		AcpiGetObjectInfo(handle, &info);
	}

	~Device()
	{
		/*
		 * [acpica] 8.3.3 AcpiGetObjectInfo
		 *
		 * The object returned from this function should be freed via ACPI_FREE.
		 */
		ACPI_FREE(info);
	}

	Name name() const
	{
		Buffer name;
		AcpiGetName(handle, ACPI_SINGLE_NAME, &name);

		return { (char const *)name.Pointer };
	}

	Pathname pathname() const
	{
		Buffer path;
		AcpiGetName(handle, ACPI_FULL_PATHNAME_NO_TRAILING, &path);

		return { (char const *)path.Pointer };
	}

	ACPI_OBJECT_TYPE type() const { return info->Type; }

	Hid hid() const
	{
		/* TODO check "The _HID object is valid only within a Full Device Descriptor." */
		if (!(info->Valid & ACPI_VALID_HID))
			return "";

		return { (char const *)info->HardwareId.String };
	}

	Cid cid() const
	{
		if (!(info->Valid & ACPI_VALID_CID))
			return "";

		/* only first compatible ID returned */
		return { (char const *)info->CompatibleIdList.Ids[0].String };
	}

	bool match(Id const &id) { return id == hid() || id == cid(); }

	bool present_ok()
	{
		Buffer retval;
		ACPI_STATUS ret =
			AcpiEvaluateObjectTyped(handle, ACPI_STRING("_STA"), nullptr,
			                        &retval, ACPI_TYPE_INTEGER);
		switch (ret) {
		case AE_NOT_FOUND:
			return true;

		case AE_OK:
			return retval.with_object([&] (auto &o) {
				unsigned const sta = o.Integer.Value;
				return sta & ACPI_STA_DEVICE_PRESENT
				    && sta & ACPI_STA_DEVICE_FUNCTIONING;
			});

		default:
			warning("AcpiEvaluateObjectTyped returned ", AcpiFormatException(ret));
		}

		return false;
	}

	ACPI_HANDLE parent() const
	{
		ACPI_HANDLE parent = nullptr;
		AcpiGetParent(handle, &parent);

		return parent;
	}

	ACPI_HANDLE child(Name const &name)
	{
		ACPI_HANDLE child = nullptr;
		AcpiGetHandle(handle, ACPI_STRING(name.string()), &child);

		return child;
	}
};


struct Resource
{
	struct Io;
	struct Irq;
	struct Gpio;
	struct Fixed_memory32;
	struct Extended_irq;
	struct I2c_serial_bus;

	template <typename> struct Address;
	using Address16 = Address<ACPI_RESOURCE_ADDRESS16>;
	using Address32 = Address<ACPI_RESOURCE_ADDRESS32>;
	using Address64 = Address<ACPI_RESOURCE_ADDRESS64>;

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

	ACPI_RESOURCE const &r;

	Resource(ACPI_RESOURCE const &r) : r(r) { }

	void print(Genode::Output &out) const;
};


struct Resource::Io
{
	ACPI_RESOURCE_IO const &r;

	Io(ACPI_RESOURCE_IO const &r) : r(r) { }

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, Right_aligned(8, "IO"));
		print(out, " ", Right_aligned(4, r.AddressLength), " ports at");
		print(out, " ", Hex(r.Minimum), "/", Hex(r.Maximum));
	}
};


struct Resource::Irq
{
	ACPI_RESOURCE_IRQ const &r;

	Irq(ACPI_RESOURCE_IRQ const &r) : r(r)
	{
		if (r.InterruptCount > 1)
			warning("Resource::Irq: only first of ", r.InterruptCount, " IRQs supported");
	}

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, Right_aligned(8, "IRQ"));
		print(out, " ", r.Interrupts[0]);
		print(out, " ", triggering(r.Triggering));
		print(out, " ", polarity(r.Polarity));
		print(out, " ", sharable(r.Sharable));
	}
};


struct Resource::Fixed_memory32 : ACPI_RESOURCE_FIXED_MEMORY32
{
	Fixed_memory32(ACPI_RESOURCE_FIXED_MEMORY32 const &res) : ACPI_RESOURCE_FIXED_MEMORY32(res) { }

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, Right_aligned(8, "FIXMEM32"));
		print(out, " ", (WriteProtect == 0 ? "RO" : "RW"));
		print(out, " ", Hex(Address), "/", Hex(AddressLength));
	}
};


template <typename BASE>
struct Resource::Address : BASE
{
	Address(BASE const &res) : BASE(res) { }

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, Right_aligned(8, "ADR16"));
		print(out, " ", BASE::ResourceType);
		print(out, " ", Right_aligned(4, BASE::Address.AddressLength), " at");
		print(out, " ", Hex(BASE::Address.Minimum), "/", Hex(BASE::Address.Maximum));

		if (BASE::ResourceSource.StringPtr)
			print(out, " ", (char const *)BASE::ResourceSource.StringPtr);
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

		print(out, Right_aligned(8, "GPIO"));
		print(out, " ", connection_type(ConnectionType));
		print(out, " ", pin_config(PinConfig));
		print(out, " ", triggering(Triggering));
		print(out, " ", polarity(Polarity));
		print(out, " ", sharable(Sharable));
		print(out, " ", producer_consumer(ProducerConsumer));
//		print(out, " PinTableLength=", PinTableLength);
		if (PinTableLength)
			print(out, " Pin0=", *PinTable);
//		print(out, " VendorLength=",   VendorLength);
		print(out, " ResourceSource=", (char const *)ResourceSource.StringPtr);
	}
};


struct Resource::I2c_serial_bus : ACPI_RESOURCE_I2C_SERIALBUS
{
	I2c_serial_bus(ACPI_RESOURCE_I2C_SERIALBUS const &res) : ACPI_RESOURCE_I2C_SERIALBUS(res)
	{
	}

	void print(Genode::Output &out) const
	{
		using Genode::print;

		print(out, Right_aligned(8, "I2C"));
		print(out, " ResourceSource=", (char const *)ResourceSource.StringPtr);
		print(out, " ", AccessMode);
		print(out, " ", SlaveAddress);
		print(out, " ", ConnectionSpeed);
	}
};


void Resource::print(Genode::Output &out) const
{
	using Genode::print;

	print(out, "{", r.Type, ",", r.Length, "} ");

	switch (r.Type) {
	case ACPI_RESOURCE_TYPE_IRQ:                print(out, Irq(r.Data.Irq));                      break;
//	case ACPI_RESOURCE_TYPE_DMA:                /* 1 */
//	case ACPI_RESOURCE_TYPE_START_DEPENDENT:    /* 2 */
//	case ACPI_RESOURCE_TYPE_END_DEPENDENT:      /* 3 */
	case ACPI_RESOURCE_TYPE_IO:                 print(out, Io(r.Data.Io));                        break;
//	case ACPI_RESOURCE_TYPE_FIXED_IO:           /* 5 */
//	case ACPI_RESOURCE_TYPE_VENDOR:             /* 6 */
//	case ACPI_RESOURCE_TYPE_END_TAG:            /* 7 */
//	case ACPI_RESOURCE_TYPE_MEMORY24:           /* 8 */
//	case ACPI_RESOURCE_TYPE_MEMORY32:           /* 9 */
//	case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:     print(out, Fixed_memory32(res.Data.FixedMemory32)); break;
//	case ACPI_RESOURCE_TYPE_ADDRESS16:          print(out, Address16(res.Data.Address16));          break;
//	case ACPI_RESOURCE_TYPE_ADDRESS32:          print(out, Address32(res.Data.Address32));          break;
//	case ACPI_RESOURCE_TYPE_ADDRESS64:          print(out, Address64(res.Data.Address64));          break;
//	case ACPI_RESOURCE_TYPE_EXTENDED_ADDRESS64: /* 14 */
//	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:       print(out, Extended_irq(res.Data.ExtendedIrq));     break;
//	case ACPI_RESOURCE_TYPE_GENERIC_REGISTER:   /* 16 */
//	case ACPI_RESOURCE_TYPE_GPIO:               print(out, Gpio(res.Data.Gpio));                    break;
//	case ACPI_RESOURCE_TYPE_FIXED_DMA:          /* 18 */
//	case ACPI_RESOURCE_TYPE_SERIAL_BUS:
//		switch (res.Data.CommonSerialBus.Type) {
//		case ACPI_RESOURCE_SERIAL_TYPE_I2C:     print(out, I2c_serial_bus(res.Data.I2cSerialBus));  break;
////		case ACPI_RESOURCE_SERIAL_TYPE_SPI:     print(out, Spi_serial_bus(res.Data.SpiSerialBus));  break;
////		case ACPI_RESOURCE_SERIAL_TYPE_UART:    print(out, Spi_serial_bus(res.Data.UartSerialBus)); break;
//		default: print(out, "unknown serial type ", res.Data.CommonSerialBus.Type); break;
//		} break;
//	case ACPI_RESOURCE_TYPE_PIN_FUNCTION:       /* 20 */
//	case ACPI_RESOURCE_TYPE_PIN_CONFIG:         /* 21 */
//	case ACPI_RESOURCE_TYPE_PIN_GROUP:          /* 22 */
//	case ACPI_RESOURCE_TYPE_PIN_GROUP_FUNCTION: /* 23 */
//	case ACPI_RESOURCE_TYPE_PIN_GROUP_CONFIG:   /* 24 */

	default:
		print(out, "unknown type ", r.Type);
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
	Device method { handle };

	log(" method ", method.name(), " - ", method.pathname(), " ", handle);

	return AE_OK;
}


ACPI_STATUS display_device(ACPI_HANDLE handle, UINT32 level,
                           void * /* context */, void ** /* retval */)
{
	Device dev { handle };

	if (!dev.present_ok()) {
		if (dev.hid() == "PNP0B00")
			warning("RTC object found but _STA returned not present/functioning");
		return AE_OK;
	}

warning(dev.pathname(), " h:", dev.hid(), " c:", dev.cid());
	AcpiWalkResources(handle, ACPI_STRING("_CRS"), detect_resource, &dev);

	ACPI_HANDLE child = NULL;
	while (true) {
		if (AcpiGetNextObject(ACPI_TYPE_METHOD, dev.handle, child, &child) != AE_OK)
			break;
		display_method(child, 0, 0, 0);
	}

	/* HID over I2C */
	if (dev.match("PNP0C50")) {
		if (false) {
			Btacpi::Buffer out;

			ACPI_STATUS ret = AcpiEvaluateObject(dev.handle, ACPI_STRING("_HID"), nullptr, &out);

			out.with_object([&] (auto &o) {
				if (o.Type != ACPI_TYPE_STRING) return;

				log("++++ _HID ret=", ret, " type=", o.Type, " string=", (void *)o.String.Pointer, "/", o.String.Length);
			});
		}

		/* call _DSM to retrieve HID descriptor address */
		do {
			ACPI_OBJECT      args[4];
			ACPI_OBJECT_LIST in { 4, args };
			Btacpi::Buffer   out;

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
			if (ACPI_FAILURE(ret)) {
				log("++++ ret=", ret);
				break;
			}

			out.with_object([&] (auto &o) {
				log(" ", Right_aligned(8, dev.hid()),
				    " ", Right_aligned(8, dev.cid()),
				    " HIDD address ", o.Integer.Value); });
		} while (false);

		/* get DW I2C bus parameters from parent */
		ACPI_HANDLE parent = dev.parent();
		auto bus_param = [&] (Name const &param) {
			Btacpi::Buffer out;

			ACPI_STATUS ret = AcpiEvaluateObjectTyped(parent, ACPI_STRING(param.string()), nullptr, &out, ACPI_TYPE_PACKAGE);
			if (ACPI_FAILURE(ret))
				return;

			out.with_object([&] (auto &o) {
				if (o.Package.Count != 3) {
					error("++++ count=", o.Package.Count);
					return;
				}

				log(" ", Right_aligned(8, dev.hid()),
				    " ", Right_aligned(8, dev.cid()),
				    " I2C ", param,
				    " ", Right_aligned(3, o.Package.Elements[0].Integer.Value),
				    " ", Right_aligned(3, o.Package.Elements[1].Integer.Value),
				    " ", Right_aligned(3, o.Package.Elements[2].Integer.Value));
			});
		};
		bus_param("SSCN");
		bus_param("FMCN");
		bus_param("FPCN");
		bus_param("HSCN");
//		bus_param("HMCN");
	}

	if (false) if (dev.match("PNP0C0B")) {
		Btacpi::Buffer out;

		ACPI_STATUS ret = AcpiEvaluateObjectTyped(dev.handle, ACPI_STRING("_FST"), nullptr, &out, ACPI_TYPE_PACKAGE);

		out.with_object([&] (auto &o) {
			log("++++ ret=", ret, " type=", (ACPI_OBJECT_TYPE)o.Package.Type, " result=", o.Package.Count);
			if (o.Package.Count >= 3)
				log(" revision:", o.Package.Elements[0].Integer.Value,
				    " control:",  o.Package.Elements[1].Integer.Value,
				    " speed:",    o.Package.Elements[2].Integer.Value);
		});
	}
	if (false) if (ACPI_HANDLE method = dev.child("_TMP")) {
		for (unsigned i = 3; i > 0; --i) {
			Btacpi::Buffer out;

			ACPI_STATUS ret = AcpiEvaluateObjectTyped(method, nullptr, nullptr, &out, ACPI_TYPE_INTEGER);

			if (ret)
				error("++++ ret=", ret);
			else
				out.with_object([&] (auto &o) {
					int temp_c = int(o.Integer.Value) - 2732; /* Kelvin to Celsius (tenth) */
					log(" _TMP ", temp_c/10, ".", temp_c%10);
				});

			AcpiOsSleep(1000);
		}
	}

	return AE_OK;
}


ACPI_STATUS display_special(ACPI_HANDLE handle, UINT32 level,
                          void * /* context */, void ** /* retval */)
{
	Device node { handle };

	log(" special ", node.name(), " - ", node.pathname(), " ", Hex(node.type()), " ", handle);

	return AE_OK;
}


static void walk_special(ACPI_STRING scope, ACPI_OBJECT_TYPE type)
{
	ACPI_HANDLE handle;

	ACPI_STATUS status = AcpiGetHandle (NULL, scope, &handle);
	if (status != AE_OK)
		return;

	log((char const *)scope);
	AcpiWalkNamespace(type, handle, ACPI_UINT32_MAX,
	                  display_special, nullptr, nullptr, nullptr);
}


void another_test()
{
//	AcpiDbgLevel |= ACPI_LV_INFO;
//	AcpiDbgLayer |= ACPI_TABLES;

	if (false) {
		Btacpi::Buffer resources;
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

	walk_special(ACPI_STRING("\\_GPE"), ACPI_TYPE_ANY);
	walk_special(ACPI_STRING("\\_PR_"), ACPI_TYPE_ANY);
//	walk_special(ACPI_STRING("\\_SB_"), ACPI_TYPE_ANY);
	walk_special(ACPI_STRING("\\_SI_"), ACPI_TYPE_ANY);
	walk_special(ACPI_STRING("\\_TZ_"), ACPI_TYPE_ANY);

//	AcpiWalkNamespace(ACPI_TYPE_METHOD, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
//	                  display_method, nullptr, nullptr, nullptr);
}

} /* namespace Btacpi */
