#include <windows.h>
#include <pxcdefs.h>
#include "Projection.hpp"
#include "DepthSensor.hpp"

ProjectionFacade::ProjectionFacade(Session* session, Capture::Device* device, const Image::ImageInfo* dinfo, const Image::ImageInfo* cinfo) {
	/* retrieve the invalid depth pixel values */
	projection = device->CreateProjection();

	uvMap.resize(dinfo->width * dinfo->height);
	invUVMap.resize(cinfo->width * cinfo->height);
	vertices.resize(dinfo->width * dinfo->height);

	Image::ImageInfo drawDepthInfo;
	memset(&drawDepthInfo, 0, sizeof(drawDepthInfo));
	drawDepthInfo.width = dinfo->width;
	drawDepthInfo.height = dinfo->height;
	drawDepthInfo.format = Image::PIXEL_FORMAT_RGB32;
	drawDepth = nullptr;
	drawDepth = session->CreateImage(&drawDepthInfo);

	Image::ImageInfo drawColorInfo;
	memset(&drawColorInfo, 0, sizeof(drawColorInfo));
	drawColorInfo.width = cinfo->width;
	drawColorInfo.height = cinfo->height;
	drawColorInfo.format = Image::PIXEL_FORMAT_RGB32;
	drawColor = nullptr;
	drawColor = session->CreateImage(&drawColorInfo);

	Image::ImageInfo drawVerticesInfo;
	memset(&drawVerticesInfo, 0, sizeof(drawVerticesInfo));
	drawVerticesInfo.width = dinfo->width;
	drawVerticesInfo.height = dinfo->height;
	drawVerticesInfo.format = Image::PIXEL_FORMAT_RGB32;
	drawVertices = nullptr;
	drawVertices = session->CreateImage(&drawVerticesInfo);
}


ProjectionFacade::~ProjectionFacade(void) {
	if (drawVertices) drawVertices->Release();
	if (drawColor) drawColor->Release();
	if (drawDepth) drawDepth->Release();
	invUVMap.clear();
	vertices.clear();
	uvMap.clear();
	if (projection) projection->Release();
}

Image* ProjectionFacade::ColorToDepthByQueryUVMap(Image *color, Image *depth)
{
	if (!drawDepth || !projection) return nullptr;

	Image::ImageInfo drawDepthInfo = drawDepth->QueryInfo();
	Image::ImageData drawDepthDat;
	if (Status::STATUS_NO_ERROR > drawDepth->AcquireAccess(Image::ACCESS_WRITE, drawDepthInfo.format, &drawDepthDat))
		return nullptr;

	/* Retrieve the color pixels */
	Image::ImageData cdata;
	Status sts = color->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGB32, &cdata);
	if (sts >= Status::STATUS_NO_ERROR) {
		sts = projection->QueryUVMap(depth, &uvMap[0]);
		if (sts >= Status::STATUS_NO_ERROR) {
			Image::ImageInfo cinfo = color->QueryInfo();
			Image::ImageInfo dinfo = depth->QueryInfo();
			for (pxcI32 y = 0; y < dinfo.height; y++) {
				pxcI32* pDrawDepthDat = reinterpret_cast<pxcI32*>(drawDepthDat.planes[0] + y * drawDepthDat.pitches[0]);
				PointF32 *uvTest = &uvMap[y * dinfo.width];
				for (pxcI32 x = 0; x < dinfo.width; x++) {
					pDrawDepthDat[x] = 0;
					if (uvTest[x].x >= 0 && uvTest[x].x < 1 && uvTest[x].y >= 0 && uvTest[x].y < 1) {
						pxcI32 tmpColor = *reinterpret_cast<pxcI32*>(cdata.planes[0] + static_cast<int>(uvTest[x].y * cinfo.height) * cdata.pitches[0] + 4 * static_cast<int>(uvTest[x].x * cinfo.width));
						pDrawDepthDat[x] = tmpColor;
					}
				}
			}
		}
		color->ReleaseAccess(&cdata);
	}
	drawDepth->ReleaseAccess(&drawDepthDat);
	return drawDepth;
}


Image* ProjectionFacade::DepthToColorByQueryInvUVMap(Image *color, Image *depth)
{
	if (!drawColor || !projection) return nullptr;

	Image::ImageInfo drawColorInfo = drawColor->QueryInfo();
	Image::ImageData drawColorDat;
	if (Status::STATUS_NO_ERROR > drawColor->AcquireAccess(Image::ACCESS_WRITE, drawColorInfo.format, &drawColorDat))
		return nullptr;

	/* Retrieve the color pixels */
	Image::ImageData ddata;
	Status sts = depth->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGB32, &ddata);
	if (sts >= Status::STATUS_NO_ERROR) {
		sts = projection->QueryInvUVMap(depth, &invUVMap[0]);
		if (sts >= Status::STATUS_NO_ERROR) {
			Image::ImageInfo cinfo = color->QueryInfo();
			Image::ImageInfo dinfo = depth->QueryInfo();
			for (pxcI32 y = 0; y < cinfo.height; y++) {
				pxcI32* pdrawColorDat = reinterpret_cast<pxcI32*>(drawColorDat.planes[0] + y * drawColorDat.pitches[0]);
				PointF32 *invUVTest = &invUVMap[y * cinfo.width];
				for (pxcI32 x = 0; x < cinfo.width; x++) {
					pdrawColorDat[x] = 0;
					if (invUVTest[x].x >= 0 && invUVTest[x].x < 1 && invUVTest[x].y >= 0 && invUVTest[x].y < 1) {
						pxcI32 tmpColor = *reinterpret_cast<pxcI32*>(ddata.planes[0] + static_cast<int>(invUVTest[x].y * dinfo.height) * ddata.pitches[0] + 4 * static_cast<int>(invUVTest[x].x * dinfo.width));
						pdrawColorDat[x] = tmpColor;
					}
				}
			}
		}
		depth->ReleaseAccess(&ddata);
	}
	drawColor->ReleaseAccess(&drawColorDat);
	return drawColor;
}


void norm(Point3DF32 &v)
{
	auto len = v.x*v.x + v.y*v.y + v.z*v.z;
	if (len > 0) {
		len = sqrt(len);
		v.x = v.x / len; v.y = v.y / len; v.z = v.z / len;
	}
}

Point3DF32 cross(Point3DF32 &v0, Point3DF32 &v1)
{
	Point3DF32 vec = { v0.y*v1.z - v0.z*v1.y, v0.z*v1.x - v0.x*v1.z, v0.x*v1.y - v0.y*v1.x };
	return vec;
}

float dot(Point3DF32 &v0, Point3DF32 &v1)
{
	auto r = 0.f;
	r += v0.x * v1.x;
	r += v0.y * v1.y;
	r += v0.z * v1.z;
	return r;
}

Image* ProjectionFacade::DepthToWorldByQueryVertices(Image *depth, Point3DF32 light)
{
	if (!drawVertices || !projection) return nullptr;

	Image::ImageInfo drawVerticesInfo = drawVertices->QueryInfo();
	Image::ImageData drawVerticesDat;
	if (Status::STATUS_NO_ERROR > drawVertices->AcquireAccess(Image::ACCESS_WRITE, drawVerticesInfo.format, &drawVerticesDat))
		return nullptr;

	/* Retrieve vertices */
	Status sts = projection->QueryVertices(depth, &vertices[0]);
	if (sts >= Status::STATUS_NO_ERROR) {
		Image::ImageInfo dinfo = depth->QueryInfo();
		pxcBYTE* pdrawVerticesDat = drawVerticesDat.planes[0];
#if 0
		const float brightness = 200.f;

		int hist, num;
		float avg, nz1, nz2;
		for (pxcI32 y = 0; y < dinfo.height - 1; y++) {
			for (pxcI32 x = 0; x < dinfo.width - 1; x++) {
				pdrawVerticesDat[4 * x] = pdrawVerticesDat[4 * x + 1] = pdrawVerticesDat[4 * x + 2] = 0;
				Point3DF32 *vertex = &vertices[y * dinfo.width + x];
				if (vertex->z) {
					hist = 255;
					nz1 = (vertex[0].z - vertex[1].z) + (vertex[dinfo.width].z - vertex[dinfo.width + 1].z);
					nz2 = (vertex[0].z - vertex[dinfo.width].z) + (vertex[1].z - vertex[dinfo.width + 1].z);
					if (nz1 > 1 || nz2 > 1) {
						hist = (int)(brightness / sqrt(abs(nz1) + abs(nz2)));
						if (hist > 255) hist = 255;
						if (hist < 0) hist = 0;
					}
					pdrawVerticesDat[4 * x] = pdrawVerticesDat[4 * x + 1] = pdrawVerticesDat[4 * x + 2] = hist;
				}
				pdrawVerticesDat[4 * x + 3] = MAXBYTE;
			}
			pdrawVerticesDat += drawVerticesDat.pitches[0];
		}
#else
		for (pxcI32 y = 1; y < dinfo.height - 1; y++) {
			for (pxcI32 x = 1; x < dinfo.width - 1; x++) {
				pdrawVerticesDat[4 * x] = pdrawVerticesDat[4 * x + 1] = pdrawVerticesDat[4 * x + 2] = 0;
				auto fLight = 0.f;
				Point3DF32 v0 = vertices[y * dinfo.width + x];
				if (v0.z != 0) {
					Point3DF32 v1 = vertices[(y - 1) * dinfo.width + (x - 1)];
					Point3DF32 v2 = vertices[(y - 1) * dinfo.width + (x + 1)];
					Point3DF32 v3 = vertices[(y + 1) * dinfo.width + (x + 1)];
					Point3DF32 v4 = vertices[(y + 1) * dinfo.width + (x - 1)];
					v1.x = v1.x - v0.x; v1.y = v1.y - v0.y; v1.z = v1.z - v0.z;
					v2.x = v2.x - v0.x; v2.y = v2.y - v0.y; v2.z = v2.z - v0.z;
					v3.x = v3.x - v0.x; v3.y = v3.y - v0.y; v3.z = v3.z - v0.z;
					v4.x = v4.x - v0.x; v4.y = v4.y - v0.y; v4.z = v4.z - v0.z;
					Point3DF32 vn1 = cross(v1, v2); norm(vn1);
					Point3DF32 vn2 = cross(v2, v3); norm(vn2);
					Point3DF32 vn3 = cross(v3, v4); norm(vn3);
					Point3DF32 vn4 = cross(v4, v1); norm(vn4);
					vn1.x += vn2.x + vn3.x + vn4.x;
					vn1.y += vn2.y + vn3.y + vn4.y;
					vn1.z += vn2.z + vn3.z + vn4.z;
					norm(vn1);
					norm(light);
					fLight = dot(vn1, light);
				}

				pdrawVerticesDat[4 * x] = pdrawVerticesDat[4 * x + 1] = pdrawVerticesDat[4 * x + 2] = pxcBYTE(abs(fLight) * 255);
				pdrawVerticesDat[4 * x + 3] = MAXBYTE;
			}
			pdrawVerticesDat += drawVerticesDat.pitches[0];
		}
#endif
	}
	drawVertices->ReleaseAccess(&drawVerticesDat);
	return drawVertices;
}


void ProjectionFacade::DepthToColor(Image *color, Image *depth, vector<PointF32> dcords, vector<PointF32> &ccords) const
{
	Image::ImageData ddata;
	if (!projection) return;
	if (Status::STATUS_NO_ERROR > depth->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_DEPTH, &ddata))
		return;
	int dsize = dcords.size();
	Point3DF32 invP3D = { -1.f, -1.f, 0.f };
	vector<Point3DF32> dcordsAbs(dsize, invP3D);
	PointF32 invP = { -1.f, -1.f };
	ccords.resize(dsize, invP);
	auto dwidth = static_cast<float>(depth->QueryInfo().width), dheight = static_cast<float>(depth->QueryInfo().height);
	for (auto i = 0; i < dsize; i++) {
		if (dcords[i].x < 0) continue;
		dcordsAbs[i].x = dcords[i].x * dwidth, dcordsAbs[i].y = dcords[i].y * dheight;
		dcordsAbs[i].z = static_cast<pxcF32>(reinterpret_cast<pxcI16*>(ddata.planes[0] + static_cast<int>(dcordsAbs[i].y) * ddata.pitches[0])[static_cast<int>(dcordsAbs[i].x)]);
	}
	depth->ReleaseAccess(&ddata);

	projection->MapDepthToColor(dsize, &dcordsAbs[0], &ccords[0]);
	auto cwidthInv = 1.f / color->QueryInfo().width, cheightInv = 1.f / color->QueryInfo().height;
	for (auto i = 0; i < dsize; i++) {
		if (ccords[i].x < 0) ccords[i].x = ccords[i].y = -1.f;
		else ccords[i].x *= cwidthInv; ccords[i].y *= cheightInv;
	}
}


void ProjectionFacade::DepthToWorld(Image *depth, vector<PointF32> dcords, vector<PointF32> &wcords) const
{
	Image::ImageData ddata;
	if (!projection) return;
	if (Status::STATUS_NO_ERROR > depth->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_DEPTH, &ddata))
		return;
	int dsize = dcords.size();
	Point3DF32 invP3D = { -1.f, -1.f, 0.f };
	vector<Point3DF32> dcordsAbs(dsize, invP3D);
	vector<Point3DF32> wcords3D(dsize, invP3D);
	PointF32 invP = { -1.f, -1.f };
	wcords.resize(dsize, invP);
	auto dwidth = static_cast<float>(depth->QueryInfo().width), dheight = static_cast<float>(depth->QueryInfo().height);
	for (auto i = 0; i < dsize; i++) {
		if (dcords[i].x < 0) continue;
		dcordsAbs[i].x = dcords[i].x * dwidth, dcordsAbs[i].y = dcords[i].y * dheight;
		dcordsAbs[i].z = static_cast<pxcF32>(reinterpret_cast<pxcI16*>(ddata.planes[0] + static_cast<int>(dcordsAbs[i].y) * ddata.pitches[0])[static_cast<int>(dcordsAbs[i].x)]);
	}
	depth->ReleaseAccess(&ddata);

	projection->ProjectDepthToCamera(dsize, &dcordsAbs[0], &wcords3D[0]);
	for (auto i = 0; i < dsize; i++) {
		if (dcordsAbs[i].z <= 0) wcords[i].x = wcords[i].y = -1.f;
		else wcords[i].x = dcords[i].x; wcords[i].y = dcords[i].y;
	}
}

void ProjectionFacade::ColorToDepth(Image *color, Image *depth, vector<PointF32> ccords, vector<PointF32> &dcords) const
{
	if (!projection) return;
	int csize = ccords.size();
	PointF32 invP = { -1.f, -1.f };
	vector<PointF32> ccordsAbs(csize, invP);
	auto cwidth = static_cast<float>(color->QueryInfo().width), cheight = static_cast<float>(color->QueryInfo().height);
	for (auto i = 0; i < csize; i++) {
		ccordsAbs[i].x = ccords[i].x * cwidth, ccordsAbs[i].y = ccords[i].y * cheight;
	}
	dcords.resize(csize);
	projection->MapColorToDepth(depth, csize, &ccordsAbs[0], &dcords[0]);
	auto dwidthInv = 1.f / depth->QueryInfo().width, dheightInv = 1.f / depth->QueryInfo().height;
	for (auto i = 0; i < csize; i++) {
		if (dcords[i].x < 0) continue;
		dcords[i].x *= dwidthInv; dcords[i].y *= dheightInv;
	}
}


void ProjectionFacade::ColorToWorld(Image *color, Image *depth, vector<PointF32> ccords, vector<PointF32> &wcords) const
{
	if (!projection) return;
	int csize = ccords.size();
	PointF32 invP = { -1.f, -1.f };
	vector<PointF32> ccordsAbs(csize, invP);
	auto cwidth = static_cast<float>(color->QueryInfo().width), cheight = static_cast<float>(color->QueryInfo().height);
	for (auto i = 0; i < csize; i++) {
		ccordsAbs[i].x = ccords[i].x * cwidth, ccordsAbs[i].y = ccords[i].y * cheight;
	}
	wcords.resize(csize);
	projection->MapColorToDepth(depth, csize, &ccordsAbs[0], &wcords[0]);
	auto dwidthInv = 1.f / depth->QueryInfo().width, dheightInv = 1.f / depth->QueryInfo().height;
	for (auto i = 0; i < csize; i++) {
		if (wcords[i].x < 0) continue;
		wcords[i].x *= dwidthInv; wcords[i].y *= dheightInv;
	}
}


void ProjectionFacade::WorldToDepth(Image *depth, vector<PointF32> wcords, vector<PointF32> &dcords) const
{
	if (!projection) return;
	Image::ImageData ddata;
	if (Status::STATUS_NO_ERROR > depth->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_DEPTH, &ddata))
		return;
	int wsize = wcords.size();
	Point3DF32 invP3D = { -1.f, -1.f, 0.f };
	vector<Point3DF32> dcordsAbs(wsize, invP3D);
	vector<Point3DF32> wcords3D(wsize, invP3D);
	PointF32 invP = { -1.f, -1.f };
	dcords.resize(wsize, invP);
	auto dwidth = static_cast<float>(depth->QueryInfo().width), dheight = static_cast<float>(depth->QueryInfo().height);
	for (auto i = 0; i < wsize; i++) {
		if (wcords[i].x < 0) continue;
		dcordsAbs[i].x = wcords[i].x * dwidth, dcordsAbs[i].y = wcords[i].y * dheight;
		dcordsAbs[i].z = static_cast<pxcF32>(reinterpret_cast<pxcI16*>(ddata.planes[0] + static_cast<int>(dcordsAbs[i].y) * ddata.pitches[0])[static_cast<int>(dcordsAbs[i].x)]);
	}
	depth->ReleaseAccess(&ddata);

	projection->ProjectDepthToCamera(wsize, &dcordsAbs[0], &wcords3D[0]);
	projection->ProjectCameraToDepth(wsize, &wcords3D[0], &dcords[0]);
	auto dwidthInv = 1.f / depth->QueryInfo().width, dheightInv = 1.f / depth->QueryInfo().height;
	for (auto i = 0; i < wsize; i++) {
		if (dcordsAbs[i].z <= 0) dcords[i].x = dcords[i].y = -1.f;
		else dcords[i].x *= dwidthInv; dcords[i].y *= dheightInv;
	}
}

void ProjectionFacade::WorldToColor(Image *color, Image *depth, vector<PointF32> wcords, vector<PointF32> &ccords) const
{
	if (!projection) return;
	Image::ImageData ddata;
	if (Status::STATUS_NO_ERROR > depth->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_DEPTH, &ddata))
		return;
	int wsize = wcords.size();
	Point3DF32 invP3D = { -1.f, -1.f, 0.f };
	vector<Point3DF32> dcordsAbs(wsize, invP3D);
	vector<Point3DF32> wcords3D(wsize, invP3D);
	PointF32 invP = { -1.f, -1.f };
	ccords.resize(wsize, invP);
	auto dwidth = static_cast<float>(depth->QueryInfo().width), dheight = static_cast<float>(depth->QueryInfo().height);
	for (auto i = 0; i < wsize; i++) {
		if (wcords[i].x < 0) continue;
		dcordsAbs[i].x = wcords[i].x * dwidth, dcordsAbs[i].y = wcords[i].y * dheight;
		dcordsAbs[i].z = static_cast<pxcF32>(reinterpret_cast<pxcI16*>(ddata.planes[0] + static_cast<int>(dcordsAbs[i].y) * ddata.pitches[0])[static_cast<int>(dcordsAbs[i].x)]);
	}
	depth->ReleaseAccess(&ddata);

	projection->ProjectDepthToCamera(wsize, &dcordsAbs[0], &wcords3D[0]);
	projection->ProjectCameraToColor(wsize, &wcords3D[0], &ccords[0]);
	auto cwidthInv = 1.f / color->QueryInfo().width, cheightInv = 1.f / color->QueryInfo().height;
	for (auto i = 0; i < wsize; i++) {
		if (dcordsAbs[i].z <= 0) ccords[i].x = ccords[i].y = -1.f;
		else ccords[i].x *= cwidthInv; ccords[i].y *= cheightInv;
	}
}


Image* ProjectionFacade::CreateDepthMappedToColor(Image *color, Image *depth) const
{
	return projection->CreateDepthImageMappedToColor(depth, color);
}

Image* ProjectionFacade::CreateColorMappedToDepth(Image *color, Image *depth) const
{
	return projection->CreateColorImageMappedToDepth(depth, color);
}

