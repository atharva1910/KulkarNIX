#pragma once

struct MemoryDescriptor{
    /* Base - Lower */
	uint32_t BaseL;
    /* Base - Higher */
	uint32_t BaseH;
    /* Length - Lower */
	uint32_t LengthL;
    /* Length - Higher */
	uint32_t LengthH;
    /* 1 - Free. 0 - Reserved */
	uint32_t Type;
    /* extended ACPI memory */
	uint32_t ACPI;
}__attribute__((packed));
