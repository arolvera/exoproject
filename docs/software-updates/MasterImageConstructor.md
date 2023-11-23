# Master Image Constructor (MIC) Utility

The MIC utility creates an update binary image file that can be used to update Halo devices. The procedure requires the following components:

| Component | Description |
| --------- | ----------- |
| MIC App   | The master image constructor app that is used to construct the update image. The source for this app is part of the Exoproject repository and built using the master_image_constructor_config.cmake file. |
| Manifest  | A file containing information about the build that is used to populate fields in the update binary header. |
| Thruster Control App | The application build to be bundled into the update image. For Halo 12 there is just one build that is deployed on all three Vorago devices. |
| Libraries | There are libraries such as libc that need to be installed on the development system to run this app. Any others ????? |

&nbsp;

## Manifest File

The manifest file is a simple text file with no extension. The structure is as follows:
```
<Input File Name>:<Comp ID>:<Magic Number>:<Git SHA>:<VersionMajor.VersionMinor.Revision>
```

- Input File Name - firmware image bin file to be packaged
- Comp ID - 0 = ECPK, 1 = MVCP, 2 = ACP
- Magic Number assigned to a device - see magic_numbers.h
- Git SHA - SHA value for the tagged release in Gitlab
- Version major, monir, and patch (revision) - see thruster_control_version.h

example:

```
system-control-halo12-va41630-xenon-silver-boeing.bin:0:786b6365:d226e385:0:1:0
```

**NOTE** Presently, the master image constructor packages just one image to be deployed for all three processors. As long as this is the case, it is only necessary to specify one firmware in the manifest and we use the ECPK device ID and magic number. The MIC is written to be able to deploy a different image for each processor. In this case, there would be three entries in the manifest, one for each device as shown in the example below:

```
system-control-halo12-va41630-xenon-silver-boeing.bin:0:786b6365:d226e385:0:1:0
system-control-halo12-va41630-xenon-silver-boeing.bin:1:7863766d:d226e385:0:1:1
system-control-halo12-va41630-xenon-silver-boeing.bin:2:78706361:d226e385:0:1:2
```

&nbsp;

# Procedure

1.  Create a build for the MIC utility if one doesn't already exist

2.  Build the Thruster control application

3.  Create a working director and copy the MIC .elf and Thruster Control .bin files into this folder

4.  Build a Manifest file as defined above and save it to the woirking directory

5.  Open a terminal and navigate to the working directory. Run the following command:

    ```
    ./<mic file>.elf -b update.bin manifest
    ```

    The output file must not already exist or else the mic app will fail.

6. Verify the image by running the following command:

    ```
    ./<mic file>.elf -i update.bin
    ```

    You should see an output similar to the following example:

    ```
    Header magic number:          0x72646875
    Header size:                  0x100
    Header number of images:      0x3
    Header crc:                   0xb9ff
    Header crc check:             GOOD

    Component:                0
    Component magic:          0x786b6365 (eckx)
    Component offset:         0x100
    Component size:           0x2a294
    Component number:         0x0
    Component version:        0.0.1.0
    Component sha:            d226e385
    Component crc:            0xe34a
    Image calc crc:           0xe34a

    Component:                1
    Component magic:          0x786b6365 (eckx)
    Component offset:         0x100
    Component size:           0x2a294
    Component number:         0x1
    Component version:        0.0.1.0
    Component sha:            d226e385
    Component crc:            0xe34a
    Image calc crc:           0xe34a

    Component:                2
    Component magic:          0x786b6365 (eckx)
    Component offset:         0x100
    Component size:           0x2a294
    Component number:         0x2
    Component version:        0.0.1.0
    Component sha:            d226e385
    Component crc:            0xe34a
    Image calc crc:           0xe34a
    ```

    Verify that the magic numbers, version strings, component ID's etc. all match the manifest inputs and that the crc's all match. Even when providing just one image, the MIC utility will create a header entry for each device.