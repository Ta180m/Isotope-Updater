// Isotope Updater
// Copyright (C) 2020 Nichole Mattera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "Action.hpp"

using namespace std;

namespace dsu::models {
    Action::Action(ActionButton actionButton, string actionText) {
        button = actionButton;
        text = actionText;
        textTexture = NULL;
        textWidth = 0;
        textHeight = 0;
    }

    Action::~Action() {
        if (textTexture != NULL) {
            SDL_DestroyTexture(textTexture);
            textTexture = NULL;
        }
    }
}