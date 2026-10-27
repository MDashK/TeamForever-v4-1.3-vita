/* vita-elf-create appends ~3KB of module info right after the code segment.
   The default script aligns the data segment to the next 64KB boundary, so depending on the
   code size there may not be enough room ("segment 1 overlaps"). Always leave 8KB free. */
SECTIONS
{
  . = . + 0x2000;
}
INSERT AFTER .eh_frame;
