# EARLY DRAFT ----- Vorago Dev Board Serial Programming Procedure

The Vorago dev board includes a USB serial port connected to a JLink chip that performs software updates. This procedure outlines steps for performing this action using a bash script.

To run the bash script you will need a Linux environment or a Windows machine with Cygwin, MinGW's MSYS, or Windows Subsystem for Linux (WSL) installed.

## Installing WSL

1) Open Windows Powershell and enter the following command:

```
wsl --install -d Ubuntu
```

2) Reboot the system to allow the changes to take effect.

3) From the Exoproject repository, copy the contents of the "tools/jlink" folder into a working directory along with the .elf file to be programmed.

4) Open Windows Powershell and start the Windows Linux Subsystem from the command line.

```
wsl
```

5) Run the bash script with the elf file as an arguement (file name will vary)

```
bash flash.sh hardwarecontrol-halo12-va41630-xenon-silver-boeing.elf
```