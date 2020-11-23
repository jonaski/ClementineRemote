<img align="left" width="80" height="80" src="https://raw.githubusercontent.com/mbruel/ClementineRemote/main/src/data/icon.png" alt="ClementineRemote">

# StrawBerry Remote (to come...)
Fork from [ClementineRemote](https://github.com/mbruel/ClementineRemote)
developped in C++/QT5/QML to be available on any OS. <br/>

<br/><br/>
<img width="200" src="https://raw.githubusercontent.com/mbruel/ClementineRemote/main/pics/ClementineRemote_Playlist.png" alt="ClementineRemote on Debian"/> <img width="200" src="https://raw.githubusercontent.com/mbruel/ClementineRemote/main/pics/ClementineRemote_Files.png" alt="ClementineRemote on iPhone SE"/> <img width="200" src="https://raw.githubusercontent.com/mbruel/ClementineRemote/main/pics/ClementineRemote_Library.png" alt="ClementineRemote on iPhone SE"/> <img width="200" src="https://raw.githubusercontent.com/mbruel/ClementineRemote/main/pics/ClementineRemote_Radio.png" alt="ClementineRemote on iPhone SE"/><br/>

## Features
### Player
- start Previous track (the one from the Active Playlist, you may be looking another one)
- start Next track (the one from the Active Playlist, you may be looking another one)
- Play/Pause or Stop current track
- change the volume
- jump to current Track (from whatever Menu or Playlist)
- change the position in the playing track
- change the reapeat mode
- change the shuffle mode
- download the playing track


### Playlist Menu
From the header Menu:
- Change Playlist (from those opened in Clementine on the server)
- search for text (regular expression for track name, album and artist)
- save/favourite a Playlist
- rename current playlist
- clear current playlist
- close current playlist
- download current playlist
- open a closed playlist that is in favourite
- create a new playlist

You can also:
- **Double click on a Track** to play it
- **Long press on a Track** to switch in **Selection mode** from where you can select more tracks or all (cf button in the bottom) to:
- Copy tracks to another Playlist
- download the selected tracks
- remove the selected tracks from the playlist.
- click on the cursor button to exit the **Selection mode**

### Files Menu
You need to have set up the **Files root folder** in Clementine settings (cf [Tools -- Preferences -- Network Remote](https://raw.githubusercontent.com/mbruel/ClementineRemote/main/pics/Clementine_Network_Remote_Settings.png))<br/>
- you can jump into a folder
- go to the parent folder
- go back to the home folder
- long press on a track or use the cursor button (bottom right) to switch in **Selection Mode**
- download selected tracks
- append selected tracks to the current Playlist
- create a new playlist with the selected tracks

### Library Menu
The library is downloaded automatically when you first log to a Clementine server
- search for text (regular expression for track name, album and artist)
- redownload the library
- download an Album or a single track
- append a Album or a single track to the current playlist
- create a new playlist with the selected Album or single track
- double click on an Artist or an Album will expand all / collapse its content

### Internet Radio Menu
The internet radio must be set on Clementine server, in the section [Internet -- Your radio streams](https://raw.githubusercontent.com/mbruel/ClementineRemote/main/pics/Clementine_Internet_Radio_Streams.png)<br/>
From **[Clementine v1.4.0rc1ClemRemote](https://github.com/mbruel/Clementine/releases/tag/1.4.0rc1ClemRemote)** or above, you can add the url of logo of your radios in Clementine :)<br/>
<br/>
**double click on a Radio** to append it to the current playlist and start to play it

### Global Search Menu
As I don't use any Internet Storage, I've not implemented this part...<br/>
If you'd be interested in it, please open a [request issue here on GitHub](https://github.com/mbruel/ClementineRemote/issues) or drop me an email at Matthieu.Bruel@gmail.com


## How to build
### Dependencies:
- build-essential (C++ compiler, libstdc++, make,...)
- qt5-default (Qt5 libraries and headers)
- qt5-qmake (to generate the moc files and create the Makefile)
- protobuf-compiler and libprotobuf-dev

### Build:
- go to `src/protobuf/` and generate the protobuf C++ file with the command `protoc remotecontrolmessages.proto --cpp_out=.`
- go to `src`
- qmake
- make



## Licence
<pre>
//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
//
// ClementineRemote is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================
</pre>


## Questions / Issues / Requests
- if you've any troubles to build or run ClemRemote, feel free to drop me an email
- if you've some comments on the code, any questions on the implementation or any proposal for improvements, I'll be happy to discuss it with you so idem, feel free to drop me an email
- if you'd like some other features, same same (but different), drop me an email ;)

Here is my email: Matthieu.Bruel@gmail.com


## Supported Languages
For now it's only in english but it is possible to add translation packages<br/>
If you'd like to translate ngPost in your language, it's easy to do (there is a nice GUI for that: QtLinguist), please get in touch with me (Matthieu.Bruel@gmail.com)


## Thanks


## Donations
I'm Freelance nowadays, working on several personal projects, so if you use the app and would like to contribute to the effort, feel free to donate what you can.<br/>
<br/>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=W2C236U6JNTUA&item_name=StrawberryRemote&currency_code=EUR"><img align="left" src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif" alt="ClementineRemote"></a>
 or in Bitcoin at this address: **3BGbnvnnBCCqrGuq1ytRqUMciAyMXjXAv6**
<img align="right" align="bottom" width="120" height="120" src="https://raw.githubusercontent.com/mbruel/ngPost/master/pics/btc_qr.gif" alt="Strawberry Remote">
