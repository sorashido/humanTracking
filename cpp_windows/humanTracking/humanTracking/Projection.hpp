#pragma once

#pragma once
#include <opencv2\core\core.hpp>
#include <vector>
#include <RealSense\Projection.h>
#include <RealSense\Session.h>
#include <RealSense\Capture.h>

using namespace std;
using namespace Intel::RealSense;

class ProjectionFacade {
public:

	ProjectionFacade(Session* session, Capture::Device* device, const Image::ImageInfo* dinfo, const Image::ImageInfo* cinfo);
	~ProjectionFacade(void);

	Image* ColorToDepthByQueryUVMap(Image *color, Image *depth);
	Image* DepthToColorByQueryInvUVMap(Image *color, Image *depth);
	Image* DepthToWorldByQueryVertices(Image *depth, Point3DF32 light);

	void DepthToColor(Image *color, Image *depth, vector<PointF32> dcords, vector<PointF32> &ccords) const;
	void DepthToWorld(Image *depth, vector<PointF32> dcords, vector<PointF32> &wcords) const;
	void ColorToDepth(Image *color, Image *depth, vector<PointF32> ccords, vector<PointF32> &dcords) const;
	void ColorToWorld(Image *color, Image *depth, vector<PointF32> ccords, vector<PointF32> &wcords) const;
	void WorldToColor(Image *color, Image *depth, vector<PointF32> wcords, vector<PointF32> &ccords) const;
	void WorldToDepth(Image *depth, vector<PointF32> wcords, vector<PointF32> &dcords) const;

	Image* CreateDepthMappedToColor(Image *color, Image *depth) const;
	Image* CreateColorMappedToDepth(Image *color, Image *depth) const;

	Image* drawDepth;
	Image* drawColor;
	Image* drawVertices;

protected:
	Projection * projection;
	vector<PointF32>   uvMap;
	vector<PointF32>   invUVMap;
	vector<Point3DF32> vertices;

	/* prohibit using copy & assignment constructors */
	ProjectionFacade(ProjectionFacade&) {}
	ProjectionFacade& operator= (ProjectionFacade&) { return *this; }
};