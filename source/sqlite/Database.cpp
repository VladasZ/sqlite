//
//  Sprite.hpp
//  sprites
//
//  Created by Vladas Zakrevskis on 7/8/19.
//  Copyright © 2019 VladasZ. All rights reserved.
//

#include "Sprite.hpp"
#include "Sprites.hpp"

using namespace gm;
using namespace sprites;

Sprite::Sprite(Image* image) : _image(image) {

}

Sprite::~Sprite() {

}

Image* Sprite::image() const {
	return _image;
}

void Sprite::update() {
	position += velocity;
}

void Sprite::draw() {
	config::drawer()->draw(this);
}
