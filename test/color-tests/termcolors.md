# Bash tips: Colors and formatting (ANSI/VT100 Control sequences)

The **ANSI/VT100** terminals and terminal emulators are not just able to display black and white text; they can display **colors** and formatted texts thanks to **escape sequences**.

Those sequences are composed of the **Escape character** (often represented by `^[` or `\e`) followed by some other characters: “`[`_FormatCode_`m`”.

In Bash, the escape character can be obtained with the following syntaxes:

* `\e`
* `\033`
* `\x1B`

_Examples:_

| Code (Bash)                                | Preview                                                                                    |
| ------------------------------------------ | ------------------------------------------------------------------------------------------ |
| ``` echo -e "\e[31mHello World\e[0m" ```   | ![Hello World](https://misc.flogisoft.com/_media/bash/colors_format/escape_example_01.png) |
| ``` echo -e "\033[31mHello\e[0m World" ``` | ![Hello World](https://misc.flogisoft.com/_media/bash/colors_format/escape_example_02.png) |

_NOTE¹:_ The `-e` option of the `echo` command enables the parsing of the escape sequences.

_NOTE²:_ The `\e[0m` sequence removes all attributes (formatting and colors). It can be a good idea to add it at the end of each colored text. ;)

_NOTE³:_ The examples on this page are in **Bash**, but the **ANSI/VT100** escape sequences can be used in every programming language.

## Formatting

Here are the most commonly supported control sequences for formatting text. Their support depends on the terminal used ([see the compatibility list](https://misc.flogisoft.com/bash/tip_colors_and_formatting#terminals_compatibility "bash:tip_colors_and_formatting")).

### Set

| Code | Description                                                                 | Example                                  | Preview                                                                                                   |
| ---- | --------------------------------------------------------------------------- | ---------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| 1    | Bold/Bright                                                                 | ``` echo -e "Normal \e[1mBold" ```       | ![Normal Bold](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_01_bright.png)           |
| 2    | Dim                                                                         | ``` echo -e "Normal \e[2mDim" ```        | ![Normal Dim](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_02_dim.png)               |
| 4    | Underlined                                                                  | ``` echo -e "Normal \e[4mUnderlined" ``` | ![Normal Underlined](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_04_underlined.png) |
| 5    | Blink [1)](https://misc.flogisoft.com/bash/tip_colors_and_formatting#fn__1) | ``` echo -e "Normal \e[5mBlink" ```      | ![Normal Blink](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_05_blink.gif)           |
| 7    | Reverse (invert the foreground and background colors)                       | ``` echo -e "Normal \e[7minverted" ```   | ![Normal inverted](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_07_inverted.png)     |
| 8    | Hidden (useful for passwords)                                               | ``` echo -e "Normal \e[8mHidden" ```     | ![Normal Hidden](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_08_hidden.png)         |

### Reset

| Code | Description          | Example                                               | Preview                                                                                                                |
| ---- | -------------------- | ----------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| 0    | Reset all attributes | ``` echo -e "\e[0mNormal Text" ```                    | ![Normal Text](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_00_reset_all.png)                     |
| 21   | Reset bold/bright    | ``` echo -e "Normal \e[1mBold \e[21mNormal" ```       | ![Normal Bold Normal](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_21_reset_bright.png)           |
| 22   | Reset dim            | ``` echo -e "Normal \e[2mDim \e[22mNormal" ```        | ![Normal Dim Normal](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_22_reset_dim.png)               |
| 24   | Reset underlined     | ``` echo -e "Normal \e[4mUnderlined \e[24mNormal" ``` | ![Normal Underlined Normal](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_24_reset_underlined.png) |
| 25   | Reset blink          | ``` echo -e "Normal \e[5mBlink \e[25mNormal" ```      | ![Normal Blink Normal](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_25_reset_blink.gif)           |
| 27   | Reset reverse        | ``` echo -e "Normal \e[7minverted \e[27mNormal" ```   | ![Normal inverted Normal](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_27_reset_inverted.png)     |
| 28   | Reset hidden         | ``` echo -e "Normal \e[8mHidden \e[28mNormal" ```     | ![Normal Hidden Normal](https://misc.flogisoft.com/_media/bash/colors_format/vt100_format_28_reset_hidden.png)         |

## 8/16 Colors

The following colors work with most terminals and terminal emulators [2)](https://misc.flogisoft.com/bash/tip_colors_and_formatting#fn__2), [see the compatibility list](https://misc.flogisoft.com/bash/tip_colors_and_formatting#terminals_compatibility "bash:tip_colors_and_formatting") for more information.

_NOTE:_ The colors can vary depending on the terminal configuration.

### Foreground (text)

| Code | Color                    | Example                                       | Preview                                                                                                            |
| ---- | ------------------------ | --------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| 39   | Default foreground color | ``` echo -e "Default \e[39mDefault" ```       | ![Default Default](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_39_default.png)             |
| 30   | Black                    | ``` echo -e "Default \e[30mBlack" ```         | ![Default Black](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_30_black.png)                 |
| 31   | Red                      | ``` echo -e "Default \e[31mRed" ```           | ![Default Red](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_31_red.png)                     |
| 32   | Green                    | ``` echo -e "Default \e[32mGreen" ```         | ![Default Green](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_32_green.png)                 |
| 33   | Yellow                   | ``` echo -e "Default \e[33mYellow" ```        | ![Default Yellow](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_33_yellow.png)               |
| 34   | Blue                     | ``` echo -e "Default \e[34mBlue" ```          | ![Default Blue](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_34_blue.png)                   |
| 35   | Magenta                  | ``` echo -e "Default \e[35mMagenta" ```       | ![Default Magenta](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_35_magenta.png)             |
| 36   | Cyan                     | ``` echo -e "Default \e[36mCyan" ```          | ![Default Cyan](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_36_cyan.png)                   |
| 37   | Light gray               | ``` echo -e "Default \e[37mLight gray" ```    | ![Default Light gray](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_37_light_gray.png)       |
| 90   | Dark gray                | ``` echo -e "Default \e[90mDark gray" ```     | ![Default Dark gray](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_90_dark_gray.png)         |
| 91   | Light red                | ``` echo -e "Default \e[91mLight red" ```     | ![Default Light red](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_91_light_red.png)         |
| 92   | Light green              | ``` echo -e "Default \e[92mLight green" ```   | ![Default Light green](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_92_light_green.png)     |
| 93   | Light yellow             | ``` echo -e "Default \e[93mLight yellow" ```  | ![Default Light yellow](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_93_light_yellow.png)   |
| 94   | Light blue               | ``` echo -e "Default \e[94mLight blue" ```    | ![Default Light blue](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_94_light_blue.png)       |
| 95   | Light magenta            | ``` echo -e "Default \e[95mLight magenta" ``` | ![Default Light magenta](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_95_light_magenta.png) |
| 96   | Light cyan               | ``` echo -e "Default \e[96mLight cyan" ```    | ![Default Light cyan](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_96_light_cyan.png)       |
| 97   | White                    | ``` echo -e "Default \e[97mWhite" ```         | ![Default White](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_fg_97_white.png)                 |

### Background

| Code | Color                    | Example                                        | Preview                                                                                                             |
| ---- | ------------------------ | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| 49   | Default background color | ``` echo -e "Default \e[49mDefault" ```        | ![Default Default](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_49_default.png)              |
| 40   | Black                    | ``` echo -e "Default \e[40mBlack" ```          | ![Default Black](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_40_black.png)                  |
| 41   | Red                      | ``` echo -e "Default \e[41mRed" ```            | ![Default Red](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_41_red.png)                      |
| 42   | Green                    | ``` echo -e "Default \e[42mGreen" ```          | ![Default Green](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_42_green.png)                  |
| 43   | Yellow                   | ``` echo -e "Default \e[43mYellow" ```         | ![Default Yellow](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_43_yellow.png)                |
| 44   | Blue                     | ``` echo -e "Default \e[44mBlue" ```           | ![Default Blue](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_44_blue.png)                    |
| 45   | Magenta                  | ``` echo -e "Default \e[45mMagenta" ```        | ![Default Magenta](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_45_magenta.png)              |
| 46   | Cyan                     | ``` echo -e "Default \e[46mCyan" ```           | ![Default Cyan](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_46_cyan.png)                    |
| 47   | Light gray               | ``` echo -e "Default \e[47mLight gray" ```     | ![Default Light gray](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_47_light_gray.png)        |
| 100  | Dark gray                | ``` echo -e "Default \e[100mDark gray" ```     | ![Default Dark gray](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_100_dark_gray.png)         |
| 101  | Light red                | ``` echo -e "Default \e[101mLight red" ```     | ![Default Light red](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_101_light_red.png)         |
| 102  | Light green              | ``` echo -e "Default \e[102mLight green" ```   | ![Default Light green](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_102_light_green.png)     |
| 103  | Light yellow             | ``` echo -e "Default \e[103mLight yellow" ```  | ![Default Light yellow](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_103_light_yellow.png)   |
| 104  | Light blue               | ``` echo -e "Default \e[104mLight blue" ```    | ![Default Light blue](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_104_light_blue.png)       |
| 105  | Light magenta            | ``` echo -e "Default \e[105mLight magenta" ``` | ![Default Light magenta](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_105_light_magenta.png) |
| 106  | Light cyan               | ``` echo -e "Default \e[106mLight cyan" ```    | ![Default Light cyan](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_106_light_cyan.png)       |
| 107  | White                    | ``` echo -e "Default \e[107mWhite" ```         | ![Default White](https://misc.flogisoft.com/_media/bash/colors_format/vt100_color_bg_107_white.png)                 |

## 88/256 Colors

Some terminals ([see the compatibility list](https://misc.flogisoft.com/bash/tip_colors_and_formatting#terminals_compatibility "bash:tip_colors_and_formatting")) can support 88 or 256 colors. Here are the control sequences that permit you to use them.

_NOTE¹:_ The color number `256` is only supported by **vte** (GNOME Terminal, XFCE4 Terminal, Nautilus Terminal, Terminator, …).

_NOTE²:_ The 88-color terminals (like **rxvt**) do not have the same color map as the 256-color terminals. To show the 88-color terminal color map, run the “[256-colors.sh](https://misc.flogisoft.com/bash/tip_colors_and_formatting#colors2 "bash:tip_colors_and_formatting")” script in an 88-color terminal.

### Foreground (text)

To use one of the 256 colors on the foreground (text color), the control sequence is “`[38;5;`_ColorNumber_`m`”, where `ColorNumber` is one of the following colors:

![XTerm 256 color list (foreground)](https://misc.flogisoft.com/_media/bash/colors_format/256_colors_fg.png)

_Examples:_

| Code (Bash)                                                                          | Preview                                                                                            |
| ------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------- |
| ``` echo -e "\e[38;5;82mHello \e[38;5;198mWorld" ```                                 | ![Hello World](https://misc.flogisoft.com/_media/bash/colors_format/256-color_fg_example_01.png)   |
| ``` for i in {16..21} {21..16} ; do echo -en "\e[38;5;${i}m#\e[0m" ; done ; echo ``` | ![Blue gradiant](https://misc.flogisoft.com/_media/bash/colors_format/256-color_fg_example_02.png) |

### Background

To use one of the 256 colors on the background, the control sequence is “`[48;5;`_ColorNumber_`m`”, where `ColorNumber` is one of the following colors:

![XTerm 256 color list (background)](https://misc.flogisoft.com/_media/bash/colors_format/256_colors_bg.png)

_Examples:_

| Code (Bash)                                                                          | Preview                                                                                            |
| ------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------- |
| ``` echo -e "\e[40;38;5;82m Hello \e[30;48;5;82m World \e[0m" ```                    | ![Hello World](https://misc.flogisoft.com/_media/bash/colors_format/256-color_bg_example_01.png)   |
| ``` for i in {16..21} {21..16} ; do echo -en "\e[48;5;${i}m \e[0m" ; done ; echo ``` | ![Blue gradiant](https://misc.flogisoft.com/_media/bash/colors_format/256-color_bg_example_02.png) |

## Attributes combination

Terminals allow attribute combinations. The attributes must be separated by a semicolon (“`;`”).

_Examples:_

| Description                             | Code (Bash)                                         | Preview                                                                                                 |
| --------------------------------------- | --------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| Bold + Underlined                       | ``` echo -e "\e[1;4mBold and Underlined" ```        | ![Bold and Underlined](https://misc.flogisoft.com/_media/bash/colors_format/combination_example_01.png) |
| Bold + Red forground + Green background | ``` echo -e "\e[1;31;42m Yes it is awful \e[0m" ``` | ![Yes it is awful](https://misc.flogisoft.com/_media/bash/colors_format/combination_example_02.png)     |

## Terminals compatibility

| Terminal | Bold | Dim | Underlined | Blink | Invert | Hidden | 8 | 16 | 88 | 256 | Comment |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| [aTerm](http://www.afterstep.org/aterm.php "http://www.afterstep.org/aterm.php")                                                                               | ok         | -         | ok      | -     | ok     | -  | ok  | ~  | -  | -  | Lighter background instead of blink.                                                                                                                      |
| [Eterm](http://www.eterm.org/ "http://www.eterm.org/")                                                                                                         | ~         | -         | ok      | -     | ok     | -  | ok  | ~  | -  | ok  | Lighter color instead of Bold. Lighter background instead of blink. Can overline a text with the “`^[[6m`” sequence.                              |
| [GNOME Terminal](http://library.gnome.org/users/gnome-terminal/ "http://library.gnome.org/users/gnome-terminal/")                                              | ok         | ok         | ok      | ok     | ok     | ok  | ok  | ok  | -  | ok  | Strikeout with the “`^[[9m`” sequence.                                                                                                            |
| [Guake](http://guake.org/ "http://guake.org/")                                                                                                                 | ok         | ok         | ok      | ok     | ok     | ok  | ok  | ok  | -  | ok  | Strikeout with the “`^[[9m`” sequence.                                                                                                            |
| [Konsole](http://konsole.kde.org/ "http://konsole.kde.org/")                                                                                                   | ok         | -         | ok      | ok     | ok     | -  | ok  | ok  | -  | ok  |                                                                                                                                                           |
| [Nautilus Terminal](https://github.com/flozz/nautilus-terminal "https://github.com/flozz/nautilus-terminal")                                                   | ok         | ok         | ok      | ok     | ok     | ok  | ok  | ok  | -  | ok  | Strikeout with the “`^[[9m`” sequence.                                                                                                            |
| [rxvt](http://rxvt.sourceforge.net/ "http://rxvt.sourceforge.net/")                                                                                            | ok         | -         | ok      | ~     | ok     | -  | ok  | ok  | ok  | -  | If the background is not set to the default color, Blink make it lighter instead of blinking. Support of italic text with the “`^[[3m`” sequence. |
| [Terminator](http://www.tenshu.net/terminator/ "http://www.tenshu.net/terminator/")                                                                            | ok         | ok         | ok      | -     | ok     | ok  | ok  | ok  | -  | ok  | Strikeout with the “`^[[9m`” sequence.                                                                                                            |
| [Tilda](http://tilda.sourceforge.net/tildaabout.php "http://tilda.sourceforge.net/tildaabout.php")                                                             | ok         | -         | ok      | ok     | ok     | -  | ok  | ok  | -  | -  | Underline instead of Dim. Convert 256-colors in 16-colors.                                                                                                |
| [XFCE4 Terminal](http://www.xfce.org/projects/terminal "http://www.xfce.org/projects/terminal")                                                                | ok         | ok         | ok      | ok     | ok     | ok  | ok  | ok  | -  | ok  | Strikeout with the “`^[[9m`” sequence.                                                                                                            |
| [XTerm](http://invisible-island.net/xterm/xterm.html "http://invisible-island.net/xterm/xterm.html")                                                           | ok         | -         | ok      | ok     | ok     | ok  | ok  | ok  | -  | ok  |                                                                                                                                                           |
| xvt                                                                                                                                                              | ok         | -         | ok      | -     | ok     | -  | -  | -  | -  | -  |                                                                                                                                                           |
| Linux TTY                                                                                                                                                        | ok         | -         | -      | -     | ok     | -  | ok  | ~  | -  | -  | Specials colors instead of Dim and Underlined. Lighter background instead of Blink, Bug with 88/256 colors.                                               |
| [VTE Terminal](http://developer.gnome.org/vte/ "http://developer.gnome.org/vte/") [3)](https://misc.flogisoft.com/bash/tip_colors_and_formatting#fn__3) | ok         | ok         | ok      | ok     | ok     | ok  | ok  | ok  | -  | ok  | Strikeout with the “`^[[9m`” sequence.                                                                                                            |

_Notations used in the table:_

* `ok`: Supported by the terminal.
* `~`: Supported in a special way by the terminal.
* `-`: Not supported at all by the terminal.

## Demonstration programs

### Colors and formatting (16 colors)

![Screenshot of the color_and_formatting.sh script](https://misc.flogisoft.com/_media/bash/colors_format/colors_and_formatting.sh.png)

The following shell script displays a lot of possible combinations of the attributes (but not all, because it uses only one formatting attribute at a time).

[colors_and_formatting.sh](https://misc.flogisoft.com/_export/code/bash/tip_colors_and_formatting?codeblock=55 "Download Snippet")

```
#!/bin/bash

# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# [http://sam.zoy.org/wtfpl/COPYING](http://sam.zoy.org/wtfpl/COPYING) for more details.

#Background
for clbg in {40..47} {100..107} 49 ; do
    #Foreground
    for clfg in {30..37} {90..97} 39 ; do
        #Formatting
        for attr in 0 1 2 4 5 7 ; do
            #Print the result
            echo -en "\e[${attr};${clbg};${clfg}m ^[${attr};${clbg};${clfg}m \e[0m"
        done
        echo #Newline
    done
done

exit 0
```

### 256 colors

![Screenshot of the 256-colors.sh script](https://misc.flogisoft.com/_media/bash/colors_format/256-colors.sh-v2.png?w=200&tok=c16bb0)

The following script displays the 256 colors available on some terminals and terminal emulators like **XTerm** and **GNOME Terminal**.

[256-colors.sh](https://misc.flogisoft.com/_export/code/bash/tip_colors_and_formatting?codeblock=56 "Download Snippet")

```
#!/bin/bash

# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# [http://sam.zoy.org/wtfpl/COPYING](http://sam.zoy.org/wtfpl/COPYING) for more details.

for fgbg in 38 48 ; do # Foreground / Background
    for color in {0..255} ; do # Colors
        # Display the color
        printf "\e[${fgbg};5;%sm  %3s  \e[0m" $color $color
        # Display 6 colors per lines
        if [ $((($color + 1) % 6)) == 4 ] ; then
            echo # New line
        fi
    done
    echo # New line
done

exit 0
```

## Links

- [Bash tips: Colors and formatting](https://misc.flogisoft.com/bash/tip_colors_and_formatting)
