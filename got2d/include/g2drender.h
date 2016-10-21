#pragma once

#include <g2dconfig.h>
#include <gmlvector.h>
#include <gmlcolor.h>

namespace g2d
{
	struct GeometryVertex
	{
		gml::vec2 position;
		gml::vec2 texcoord;
		gml::color4 vtxcolor;
	};
}
