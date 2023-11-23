This is the README on configuring SAMV71 apps and bootloaders.

#Configs for directory walking:
EXOPROJECT_APP_TYPE "exoapps"         // Always set as "exoapps", as the appilcations and bootloaders live under exoapps in the project structure
EXOPROJECT_APP "fcp","halo6"  // "fcp" = configurations for flight control processor (TPC), "halo6" = configurations for halo6 control processor
EXODRIVER_PROCESSOR "samv71"      // Always set as "samv71".  FCP and halo6 applications are only supported on samv71 processors right not

#Application Configs:
BUILD_PROJECT_BOOTLOADER OFF,ON   // ON will build the bootloader associated with the EXOPROJECT_APP_TYPE, EXOPROJECT_APP and EXODRIVER_PROCESSOR settings.  OFF will build the associated appilcation.

#Client control configs:
TODO: Build control configs based on EXOPROJECT_APP setting
EXOMOD_CTRL_ANODE  "hv", "lv" // Configure Anode in high or low voltage configuration
EXOMOD_CTRL_KEEPER "hv", "lv" // Configure Keeper in high or low voltage configuration
EXOMOD_CTRL_MAGNET "hv", "lv" // Configure Magnet in high or low voltage configuration

#Project details:
EXOMOD_GAS      "na","xenon"           // Set the gas to be used for the application
EXOMOD_COIL     "na","silver","bronze" // Set the coil material used
EXOMOD_CUSTOMER "tpc","boeing","bct"   // Set the intended customer