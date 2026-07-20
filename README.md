# LPCalibrator (HGTCalibrateService)

Location Processor calibration bias engine. Matches raw 406 MHz beacon
detections against known reference beacons (`DBF_ReferenceBeacons.csv`) and
computes per-antenna TOA/FOA bias corrections, consumed by the Location
Processor via an internal EMS packet pipeline
(`LEO_CALIBRATE_406_2_DATA` / `MEO_CALIBRATE_406EX_DATA`).

## Build prerequisites

`Common/` (the shared EMS utility library) is vendored into this repo, so no
extra setup is needed for it.

The following are **not** vendored here and must be available at the paths
referenced by `HGTCalibrateService/HGTCalibrateService.vcxproj`
(`AdditionalIncludeDirectories` / `AdditionalLibraryDirectories`) for a full
build:

- Apache Xerces-C (`3rdParty/Apache/xerces/c/libx86/`)
- XSD-3.3 (`3rdParty/XSD-3.3/`)
- Microsoft MSXML 4.0 SP3 SDK (`3rdParty/Microsoft/msxml4_SP3/`)

These are the same versions already vendored under `3rdParty/` in the
DBFCollectorMT repo — point the project at a copy of that folder (placed two
directories up from `HGTCalibrateService.vcxproj`, i.e. sibling to this
repo's own root) or adjust the include/library paths to wherever they're
kept locally.

Two additional referenced paths (`../../include` and
`../../ConfigClientCPP/Debug`) don't currently resolve in this environment
either; the actual `EMSConfigClientCPP.lib` dependency is already satisfied
by the vendored `Common/Library/`, so these appear to be stale/redundant
search paths rather than hard requirements.
