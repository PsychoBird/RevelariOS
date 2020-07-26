# RevelariOS - "To Reveal - for iOS"

RevelariOS is a CLI memory scanner meant for the average user. SearchKit (searchkit.h) is also included for developers to use to add memory searching in their own projects.

## RevelariOS CLI Features:

- search bytes from memory (search)
- search string from memory (searchstr)
- read the memory in a hex view around the address returned by the scan

Make sure to sign RevelariOS with ent.xml! RevelariOS can't operate without proper entitlements.

### Signing RevelariOS

Use `ldid` for signing RevelariOS. Exactly as typed and as root type `ldid -Sent.xml RevelariOS`

![RevelariOS](revelarios.png)

## Docs - searchkit.h

`kern_return_t get_region_size(mach_port_t task, vm_address_t *baseaddr, vm_address_t *endaddr)`
- Gets memory region for searching
- **mach_port_t task** - task for the process that will be searched
- **vm_address_t \*baseaddr** - base address found by get_region_size (out)
- **vm_address_t \*endaddr** - end address found by get_region_size (out)
- **RETURN** - KERN_SUCCESS / KERN_FAILURE

`search_t search_data(mach_port_t task, bool isString, vm_address_t baseaddr, vm_address_t end, vm_address_t *outaddr, char in[100])`
- Searches for the provided data
- **mach_port_t task** - task for the process that will be searched
- **bool isString** - boolean for wether the input data is bytes or simply a string
- **vm_address_t baseaddr** - base address for searching. Use `*baseaddr` from `get_region_size`
- **vm_address_t end** - base address for searching. Use `*endaddr` from `get_region_size`
- **vm_address_t  \*outaddr** - address where the searched data is found (out)
- **char in[100]** - the data to be searched. if `isString = true`, `search_data` will search for a string. The input string should be `"ABCDE"`. If `isString = false`, `search_data` will search for the provided bytes. The input bytes should be `"6269742e6c792f3368476634696d"`
- **RETURN** - SEARCH_SUCCESS / SEARCH_FAILURE / BYTES_UNEVEN

### CONSTANTS

`SEARCH_SUCCESS (0)` - search was successful
`SEARCH_FAILURE (1)` - search failed
`BYTES_UNEVEN (2)` - bytes attempted to be searched were uneven: ex: "41b". Use "410b" instead

### Typedefs

`search_t (int)` 
`byte_t (unsigned char)`
