#pragma once

#include "agg_path_storage.h"

class agghelper
{
public:
	
	static agg::path_storage RectanglePath(double left, double right, double top, double bottom)
	{
		agg::path_storage path;
		path.move_to(left,top);
		path.line_to(right,top);
		path.line_to(right,bottom);
		path.line_to(left,bottom);
		path.line_to(left,top);
		return path;
	}
	
};
