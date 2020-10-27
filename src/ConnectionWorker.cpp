//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// Code reused from clementine-player project: https://github.com/clementine-player/Clementine
//
// This file is a part of ClementineRemote : https://github.com/mbruel/ClementineRemote
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

#include "ConnectionWorker.h"
#include "ClementineRemote.h"
#include "player/RemotePlaylist.h"

ConnectionWorker::ConnectionWorker(ClementineRemote *remote, QObject *parent) :
    QObject(parent),
    _remote(remote),
    _allow_downloads(false), _downloader(false),
    _socket(nullptr), _timeout(), _disconnectReason(" "),
    _reading_protobuf(false), _expected_length(0), _buffer(),
    _host(), _port(0), _auth_code(-1)
{
    connect(_remote, &ClementineRemote::connectToServer,      this, &ConnectionWorker::onConnectToServer,      Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::disconnectFromServer, this, &ConnectionWorker::onDisconnectFromServer, Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::changeToSong,         this, &ConnectionWorker::onChangeToSong,         Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::setTrackPostion,      this, &ConnectionWorker::onSetTrackPostion,      Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::setVolume,            this, &ConnectionWorker::onSetVolume,            Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::nextSong,             this, &ConnectionWorker::onNextSong,             Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::previousSong,         this, &ConnectionWorker::onPreviousSong,         Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::setEngineState,       this, &ConnectionWorker::onSetEngineState,       Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::shuffle,              this, &ConnectionWorker::onShuffle,              Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::repeat,               this, &ConnectionWorker::onRepeat,               Qt::QueuedConnection);
    connect(_remote, &ClementineRemote::changePlaylist,       this, &ConnectionWorker::onChangePlaylist,       Qt::QueuedConnection);

    connect(&_timeout, &QTimer::timeout, this, &ConnectionWorker::onSocketTimeout);

    _buffer.setData(QByteArray());
    _buffer.open(QIODevice::ReadWrite);
}

ConnectionWorker::~ConnectionWorker()
{
    if (_socket)
    {
        disconnect(_socket, &QAbstractSocket::disconnected, this, &ConnectionWorker::onDisconnected);
        disconnect(_socket, &QIODevice::readyRead,          this, &ConnectionWorker::onReadyRead);
        _socket->disconnectFromHost();
        if (_socket->state() != QAbstractSocket::UnconnectedState)
            _socket->waitForDisconnected();
        delete _socket;
    }
}


void ConnectionWorker::onConnectToServer(const QString &host, ushort port, int auth_code)
{
    _host      = host;
    _port      = port;
    _auth_code = auth_code;
    _socket = new QTcpSocket();

//    _socket->moveToThread(&_thread);

    _socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
    _socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    //    _socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, NgPost::articleSize());

    connect(_socket, &QAbstractSocket::connected,    this, &ConnectionWorker::onConnected,    Qt::DirectConnection);
    connect(_socket, &QAbstractSocket::disconnected, this, &ConnectionWorker::onDisconnected, Qt::DirectConnection);
    connect(_socket, &QIODevice::readyRead,          this, &ConnectionWorker::onReadyRead,    Qt::DirectConnection);

    qRegisterMetaType<QAbstractSocket::SocketError>("SocketError" );
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    connect(_socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)), Qt::DirectConnection);

    _socket->connectToHost(host, port);
    _timeout.start(_remote->sockTimeoutMs());
}

void ConnectionWorker::onDisconnectFromServer()
{
    _socket->disconnectFromHost();
}

void ConnectionWorker::onNextSong()
{
    qDebug() << "[ConnectionWorker::onNextSong]";
    onChangeToSong(1 + static_cast<int>(_remote->currentSongIndex()));
}

void ConnectionWorker::onPreviousSong()
{
    qDebug() << "[ConnectionWorker::onPreviousSong]";
    int currentIndex = static_cast<int>(_remote->currentSongIndex()) - 1;
    if (currentIndex > 0 )
        onChangeToSong(currentIndex);
}

void ConnectionWorker::onSetEngineState(qint32 state)
{
    qDebug() << "[ConnectionWorker::onSetEngineState] state: " << state;

    pb::remote::Message msg;
    switch (static_cast<pb::remote::EngineState>(state)) {
      case pb::remote::EngineState::Playing:
        msg.set_type(pb::remote::PLAY);
        break;
      case pb::remote::EngineState::Paused:
        msg.set_type(pb::remote::PAUSE);
        break;
      case pb::remote::EngineState::Empty:
        msg.set_type(pb::remote::STOP);  // Empty is called when player stopped
        break;
      default:
        msg.set_type(pb::remote::STOP);
        break;
    }


    sendDataToServer(msg);

    // Hack to make sure last played song will be resumed
    if (_remote->playerPreviousState() == pb::remote::EngineState::Idle && msg.type() == pb::remote::PLAY )
    {
        qDebug() << "[ConnectionWorker::onSetEngineState] try to play previous song: " << _remote->currentSong().title;
        _doChangeSong(static_cast<int>(_remote->currentSongIndex()), _remote->activePlaylistID());
    }

}

void ConnectionWorker::onShuffle(ushort mode)
{
    pb::remote::Message msg;
    msg.set_type(pb::remote::SHUFFLE);

    switch (mode) {
      case 0:
        msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_Off);
        break;
      case 1:
        msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_All);
        break;
      case 2:
        msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_InsideAlbum);
        break;
      case 3:
        msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_Albums);
        break;
    }

    sendDataToServer(msg);
}

void ConnectionWorker::onRepeat(ushort mode)
{
    pb::remote::Message msg;
    msg.set_type(pb::remote::REPEAT);

    switch (mode) {
      case 0:
        msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Off);
        break;
      case 1:
        msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Track);
        break;
      case 2:
        msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Album);
        break;
      case 3:
        msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Playlist);
        break;
//      case PlaylistSequence::Repeat_OneByOne:
//        msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_OneByOne);
//        break;
//      case PlaylistSequence::Repeat_Intro:
//        msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Intro);
//        break;
    }

    sendDataToServer(msg);
}

void ConnectionWorker::onChangePlaylist(qint32 pIdx)
{
    qDebug() << "[ConnectionWorker::onChangePlaylist] playlistIdx: " << pIdx;

    pb::remote::Message msg;
    msg.set_type(pb::remote::REQUEST_PLAYLIST_SONGS);
    msg.mutable_request_playlist_songs()->set_id(_remote->playlist(pIdx)->id);

    sendDataToServer(msg);
}

void ConnectionWorker::_doChangeSong(int songIndex, qint32 playlistID)
{
    const RemoteSong &song = _remote->playlistSong(songIndex);

    pb::remote::Message msg;
    msg.set_type(pb::remote::CHANGE_SONG);
    pb::remote::RequestChangeSong *request = msg.mutable_request_change_song();
    if (playlistID != -1)
        request->set_playlist_id(playlistID);

    qDebug() << "[ConnectionWorker::onChangeToSong] idx: " << songIndex << " : "<< song.str();

    request->set_song_index(song.index);

    sendDataToServer(msg);
}

void ConnectionWorker::onChangeToSong(int proxyRow)
{
#ifdef __USE_PLAYLIST_PROXY_MODEL__
    int modelRow = _remote->modelRowFromProxyRow(proxyRow);
    if (modelRow != -1)
    {
        _doChangeSong(modelRow, _remote->currentPlaylistID());

        // this will play music so we need to update the remote
        _remote->setPlay();
        emit _remote->updateEngineState();
    }
    else
        qCritical() << "[ConnectionWorker::onChangeToSong] ERROR getting model row from proxy one: " << proxyRow;
#else
    _doChangeSong(proxyRow, _remote->currentPlaylistID());
    // this will play music so we need to update the remote
    _remote->setPlay();
    emit _remote->updateEngineState();
#endif
}

void ConnectionWorker::onSetTrackPostion(qint32 newPos)
{
    pb::remote::Message msg;
    msg.set_type(pb::remote::SET_TRACK_POSITION);
    msg.mutable_request_set_track_position()->set_position(newPos);

    qDebug() << "[ConnectionWorker::onSetTrackPostion] new Pos: " << newPos
             << " ("<< _remote->prettyLength(newPos) << " for track: " << _remote->currentSong().title;

    sendDataToServer(msg);
}

void ConnectionWorker::onSetVolume(qint32 vol)
{
    pb::remote::Message msg;
    msg.set_type(pb::remote::SET_VOLUME);
    msg.mutable_request_set_volume()->set_volume(vol);
    qDebug() << "[ConnectionWorker::onSetVolume] new volume: " << vol;
    sendDataToServer(msg);
}

void ConnectionWorker::onConnected()
{
    _timeout.stop();

    _remote->saveSettings(_host, QString::number(_port), QString::number(_auth_code));

    qDebug() << "[ConnectionWorker::onConnected] to server " << _host << ":" << _port;
    // Create the general message and set the message type
    pb::remote::Message msg;
    msg.set_type(pb::remote::CONNECT);

    pb::remote::RequestConnect *reqConnect = msg.mutable_request_connect();
    if (_auth_code != -1)
        reqConnect->set_auth_code(_auth_code);

    sendDataToServer(msg);

//    emit _remote->connected();
}

void ConnectionWorker::onDisconnected()
{
    qDebug() << "[ConnectionWorker::onDisconnected] from server " << _host << ":" << _port;
    if (_timeout.isActive())
        _timeout.stop();

    disconnect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)));
    disconnect(_socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)));
    disconnect(_socket, &QIODevice::readyRead, this, &ConnectionWorker::onReadyRead);


    _socket->deleteLater();
    _socket = nullptr;

    emit _remote->disconnected(_disconnectReason);
}

void ConnectionWorker::onReadyRead()
{
    while (_socket->bytesAvailable()) {
        if (!_reading_protobuf) {
            // If we have less than 4 byte, we cannot read the length. Wait for more
            // data
            if (_socket->bytesAvailable() < 4) {
                break;
            }
            // Read the length of the next message
            QDataStream s(_socket);
            s >> _expected_length;

            // Receiving more than 128mb is very unlikely
            // Flush the data and disconnect the client
            if (_expected_length > 134217728) {
                qDebug() << "Received invalid data, disconnect client";
                qDebug() << "_expected_length =" << _expected_length;
                _socket->close();
                return;
            }

            _reading_protobuf = true;
        }

        // Read some of the message
        _buffer.write(_socket->read(_expected_length - _buffer.size()));

        // Did we get everything?
        if (_buffer.size() == _expected_length) {
            // Parse the message
            _remote->parseMessage(_buffer.data());

            // Clear the buffer
            _buffer.close();
            _buffer.setData(QByteArray());
            _buffer.open(QIODevice::ReadWrite);
            _reading_protobuf = false;
        }
    }
}

void ConnectionWorker::onSocketTimeout()
{
    qDebug() << "[ConnectionWorker::onSocketTimeout] to server " << _host << ":" << _port;
    _timeout.stop();
    emit _remote->connectionError(tr("Unable to connect..."));
    if (_socket)
        onDisconnected();
}

void ConnectionWorker::onError(QAbstractSocket::SocketError err)
{
    qDebug() << "[ConnectionWorker::onError] err: " << err << " : " << _socket->errorString();
    emit _remote->connectionError(_socket->errorString());

    if (err == QAbstractSocket::SocketError::ConnectionRefusedError)
    {
        _disconnectReason = _socket->errorString();
        _timeout.stop();
        onDisconnected();
    }
}



void ConnectionWorker::sendDataToServer(pb::remote::Message &msg)
{
    // Set the default version
    msg.set_version(msg.default_instance().version());

    // Check if we are still connected
    if (_socket->state() == QTcpSocket::ConnectedState) {
        // Serialize the message
        std::string data = msg.SerializeAsString();

        // write the length of the data first
        QDataStream s(_socket);
        s << qint32(data.length());
        if (_downloader) {
            // Don't use QDataSteam for large files
            _socket->write(data.data(), static_cast<qint64>(data.length()));
        } else {
            s.writeRawData(data.data(), static_cast<int>(data.length()));
        }

        // Do NOT flush data here! If the client is already disconnected, it
        // causes a SIGPIPE termination!!!
    } else {
        qDebug() << "Closed";
        _socket->close();
    }
}





