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

#pragma once

#include <list>
#include <SDL2/SDL.h>
#include <switch.h>

#include "View.hpp"

namespace ku {
    class Scene {
        public:
            Scene();
            virtual ~Scene(){};

            virtual void handleButton(u32 buttons, double dTime){};
            virtual void render(SDL_Rect rect, double dTime);

            /* Touch Controls */
            void touchStarted();
            void touchMoved();
            void touchEnded();

            /* View Hierarchy */
            std::list<View *> subviews;
            void addSubView(View * view);
            void removeSubView(View * view);

        private:
            View * _touchedView;
    };
}
