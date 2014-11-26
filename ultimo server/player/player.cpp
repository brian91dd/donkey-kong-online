#include "player.hpp"

Player::Player() {
	this->vidas = 0;
	this->paulines = 0;
	setEstado(Protocolo::WAITING);
	this->eliminado = false;
}

Player::Player(string name) {
	this->name = name;
	this->vidas = 0;
	this->paulines = 0;
	setEstado(Protocolo::WAITING);
	this->eliminado = false;
}

Player::Player(Player *player) {
	this->name = player->getName();
	this->sockFd = player->getSockFd();
	this->vidas = player->getVidas();
	this->paulines = player->getPaulines();
	this->estado = player->getEstado();
	this->eliminado = false;
}


string Player::getName() {
	return this->name;
}
void Player::setName(string name) {
	this->name = name;
}

int Player::getSockFd() {
	return this->sockFd;
}
void Player::setSockFd(int sockFd) {
	this->sockFd = sockFd;
}

int Player::getVidas() {
	return this->vidas;
}
int Player::getPaulines() {
	return this->paulines;
}
int Player::getPaulinesTotales() {
	return this->paulinesTotales;
}
bool Player::isPlaying() {
	return (this->estado == Protocolo::PLAYING ? true : false);
}
bool Player::isEliminated() {
	return (this->estado == Protocolo::LOSE ? true : false);
}

void Player::setEstado(string estado) {
	this->estado = estado;
}
string Player::getEstado() {
	return this->estado;
}
void Player::setVidas(int cantVidas) {
	this->vidas = cantVidas;
}
void Player::setPaulines(int cantPaulines) {
	this->paulines = cantPaulines;
}

/*Player& Player::operator=(Player p) {
	swap(p);
	return *this;
}*/