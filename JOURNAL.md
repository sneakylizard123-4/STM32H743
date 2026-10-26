---
title: STM32H743 FPGA Devboard
author: sneakylizard123-4
description: 6-layer STM32H743 board with an iCE40 UP5K, USB High-Speed, Ethernet, and audio I/O.
created_at: 2026-10-25
---

# October 25: Submission review and BOM

## What I did:

- Exported the PCB renders, copper composite, and all 14 schematic pages.
- Built a priced BOM with LCSC and Mouser links for every part.
- Went through the design against the Forge submission checklist.
- Found four design problems: the LTDC sheet is still a placeholder, X2 uses a 3225 4-pad footprint for a 32.768 kHz crystal, C1 was 120 uF in an 0603 footprint, and the exact iCE40 part is obsolete.
- Wrote the README and listed the open issues.

## Why:

- The KiCad files existed but there was no README, BOM, license, or journal for someone else to review.
- I want the review to be about the real board, not a cleaned-up version that ignores the parts that do not work yet.
- The X2 and C1 problems have to be fixed in a rev B before the board can be assembled as drawn.

## Screenshots:

![PCB and copper composite](images/screenshots/2026-1025-1132-pcb.png)

![LTDC placeholder sheet](images/screenshots/2026-1025-1132-11-ltdc.png)

![Power schematic](images/screenshots/2026-1025-1132-02-power.png)

**Total time spent: 2 hours**
