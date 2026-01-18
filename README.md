
# Rosé Patcher
<img width="1000" height="250" alt="Rosé Patcher by Project Rosé (README banner)" src="https://github.com/user-attachments/assets/71c9b5e2-b006-4153-9992-f524a289038f" />

[![GitHub release](https://img.shields.io/github/release/Project-Rose/RosePatcher.svg)](https://github.com/Project-Rose/RosePatcher/releases) [![Github all releases](https://img.shields.io/github/downloads/Project-Rose/RosePatcher/total.svg)](https://gitHub.com/Project-Rose/RosePatcher/releases/) [![GitHub license](https://badgen.net/github/license/Project-Rose/RosePatcher)](https://github.com/Project-Rose/RosePatcher/blob/master/LICENSE) [![Services Health](https://projectrose.montastic.io/badge)](https://projectrose.montastic.io) 

This is Rosé Patcher! The Aroma plugin that revives Nintendo TVii icon from the Wii U Menu and HOME Menu, as well as the applet from inside!

> [!NOTE]
> Please note that some features in the TVii revival may not be ready, as it is currently in a beta state. For more information, please check out our website at https://projectrose.cafe/ and/or our Discord server at https://discord.gg/AaTsXndGun for more up-to-date information about our services!

# Installation
The easiest way to install Rosé Patcher is to download it from the [HB App Store](https://hb-app.store/) directly on the console!

[<img width="1080" height="300" alt="HB App Store shortcut link" src="https://github.com/user-attachments/assets/b665a650-9f9b-4db0-a980-c326a1da74b5" style="width:25%; height:auto;" />](https://hb-app.store/wiiu/RosePatcher)

Alternatively, you can install it manually. Download the [latest release](https://github.com/Project-Rose/RosePatcher/releases) and place `RosePatcher.wps` in the Aroma plugins folder on your SD card (usually `sd:/wiiu/environments/aroma/plugins`).

# Configuration
Hold L + Down + Start to open the Wii U Plugin Menu, then scroll down and click Rosé Patcher. From here, there are several things that you can configure:
- **Connect to Rosé** - Patches the TVii applet to connect to Project Rosé's TVii revival.
- **Add TVii icon to the HOME Menu** - Adds the TVii icon to the HOME Menu bar on the bottom of the screen.
- **Add TVii icon to the Wii U Menu** - Adds the TVii icon to Wii U Menu suspend bar on the bottom of the screen. Your console will restart if you change this setting.

An Aroma notification should tell you if the patch is enabled every time you boot up your console.

# Credits
- [Cooper](https://github.com/coopeeo) ([@coopeeo](https://github.com/coopeeo)): Lead Plugin Developer - TVii Config, Acquire Independent Service Token (absolute hell), and plugin settings.
- [Swirlz](https://github.com/itzswirlz) ([@ItzSwirlz](https://github.com/itzswirls)): Plugin Developer - Helped a lot with issues that the person above had.
- [darcellapy](https://github.com/darcellapy) ([@darcellapy](https://github.com/darcellapy)): For the original [Vino Config Patcher](https://github.com/darcellapy/vino-config-patcher-plugin).
- [Pretendo Network](https://pretendo.network) and [Ash](https://github.com/ashquarky) ([@ashquarky](https://github.com/ashquarky)): for CI/Building with GitHub Actions. (Note: we are **not** affiliated with Pretendo, nor do they endorse us. We simply took the things that Ash did and replicated it here).
- All the people in the Aroma Discord Server (to help the developers understand what they are doing).
- All the people who helped worked on this project, you can see them at https://projectrose.cafe; also any others that are not mentioned.

# Common Issues

## My console is crashing at the Wii U Menu! I also have a Japan console!
We were not able to detect your console region automaticly. On your SD card, go to `sd:/wiiu/environments/aroma/plugins/config`, and then open the the file named `rosepatcher.json`. After you have opened the file, change the `force_jpn_console` value from `false` to `true`.

> [!NOTE]
> If you need more support, or have any questions that are not answered here, please ask in our [Discord server](https://discord.gg/AaTsXndGu)!



