# Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
#
# This file is part of Arx Libertatis.
#
# Arx Libertatis is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Arx Libertatis is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Arx Libertatis. If not, see <http://www.gnu.org/licenses/>.

from mathutils import Vector

class ArxException(Exception):
    """Common exception thrown by this addon"""
    pass

class InconsistentStateException(Exception):
    """Thrown if data supposed to be added to existing data does not match"""
    pass

def arx_pos_to_blender_for_model(pos):
    """x=>x; y=>-z; z=>y"""
    return Vector((pos[0], pos[2], -pos[1]))

def blender_pos_to_arx(pos):
    return (pos[0], -pos[2], pos[1])
