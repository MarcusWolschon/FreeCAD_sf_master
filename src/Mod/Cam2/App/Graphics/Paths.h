
#pragma once

#include <PreCompiled.h>

#include <QString>

#include <TopoDS_Edge.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax2.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <gp_Pln.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <Bnd_Box.hxx>
#include <gp_Circ.hxx>

#include <set>
#include <string>

#include "Area.h"
#include "Feature.h"
#include "TPGFeature.h"

namespace Cam
{
	double GetTolerance();

	typedef std::list<TopoDS_Wire> Wires_t;
	typedef std::list<TopoDS_Face> Faces_t;

	/**
		Define a class that encapsulates a coordinate and its own coordinate system.

		This allows the application to add/subtract offsets from each axis and the
		actual point's location will change based on that adjustment within the
		context of the coordinate system (i.e. rotation)  It's really just about
		making the application layer a little easier to read.

		eg: gp_Ax2 ax2(......)
			Point point(ax2);
			point.XAdjustment( 3.4 );	// Add 3.4 to the current X coordinate.  This change
										// may well result in an adjustment to any/all of the
										// X,Y or Z values based on the rotation of ax2.
			gp_Pnt final_location( point.Location() );
	 */
	class CamExport Point : public gp_Ax2
	{
	public:
		Point(const gp_Ax2 ax2);
		Point(const gp_Pnt p);
		Point(const double x, const double y, const double z);
		Point(const gp_Pln plane, const double x, const double y, const double z);
		Point(const gp_Pln plane);
		Point(const Point & rhs);
		Point & operator= ( const Point & rhs );
		gp_Pnt Location() const;
		Point & Location( const gp_Pnt p );

	public:
		Point & Adjustment( const double x_offset, const double y_offset, const double z_offset );
		Point & operator+= ( const gp_XYZ offset );
		Point & Adjustment( const gp_Dir direction, const double distance );

		Point & XAdjustment( const double offset );
		Point & YAdjustment( const double offset );
		Point & ZAdjustment( const double offset );

		Point & SetX( const double value );
		Point & SetY( const double value );
		Point & SetZ( const double value );

	public:
		double X(const bool in_drawing_units = false) const;
		double Y(const bool in_drawing_units = false) const;
		double Z(const bool in_drawing_units = false) const;

		double Units() const;
		double Tolerance() const;

		bool operator==( const Point & rhs ) const;
		bool operator!=( const Point & rhs ) const;
		bool operator<( const Point & rhs ) const;

		void ToDoubleArray( double *pArrayOfThree ) const;

		double XYDistance( const Point & rhs ) const;
		double XZDistance( const Point & rhs ) const;
		double YZDistance( const Point & rhs ) const;

	private:
		static double s_tolerance;
	}; // End Point class definition

	/**
		By defining a structure that inherits from std::binary_function and has an operator() method, we
		can use this class to sort lists or vectors of Cam::Point objects.  We will do this, initially, to
		sort points of NC operations so as to minimize rapid travels.

		The example code to call this would be;
			std::vector<Cam::Point> points;		// Some container of Cam::Point objects
			points.push_back(Cam::Point(3,4,5));	// Populate it with good data
			points.push_back(Cam::Point(6,7,8));

			for (std::vector<Cam::Point>::iterator l_itPoint = points.begin(); l_itPoint != points.end(); l_itPoint++)
			{
				std::vector<Cam::Point>::iterator l_itNextPoint = l_itPoint;
				l_itNextPoint++;

				if (l_itNextPoint != points.end())
				{
					sort_points_by_distance compare( *l_itPoint );
					std::sort( l_itNextPoint, points.end(), compare );
				} // End if - then
			} // End for
	 */
	struct sort_points_by_distance : public std::binary_function< const Cam::Point &, const Cam::Point &, bool >
	{
		sort_points_by_distance( const Cam::Point reference_point ) : m_reference_point(reference_point)
		{
		} // End constructor

		Cam::Point m_reference_point;

		// Return true if dist(lhs to ref) < dist(rhs to ref)
		bool operator()( const Cam::Point & lhs, const Cam::Point & rhs ) const
		{
			return( lhs.Location().Distance( m_reference_point.Location() ) < rhs.Location().Distance( m_reference_point.Location() ) );
		} // End operator() overload
	}; // End sort_points_by_distance structure definition.



	std::set<Cam::Point> Corners(Bnd_Box box);
	Standard_Real LargestBoxDimension(Bnd_Box box);


	/**
		Define an Ax3 class simply to provide an equivalence
		operator for the gp_Ax3 class.  If we don't need to rotate
		a Path from one plane to another then we should avoid doing so.  It's
		expensive in terms of CPU time to do so.  If the path's plane
		is the same as the XY plane anyway then there's no need to adjust it.
	 */
	class CamExport Ax3 : public gp_Ax3
	{
	public:
		Ax3( const gp_Ax3 & rhs );
		bool operator== ( const Ax3 & rhs ) const;
		bool operator!= ( const Ax3 & rhs ) const;
	};

	CamExport bool IsValid(const TopoDS_Shape shape);	// is non-null and has length > tolerance.
	CamExport Standard_Real Area(const TopoDS_Shape shape);
	CamExport std::list<TopoDS_Face> IntersectFaces( const TopoDS_Face lhs, const TopoDS_Face rhs );
	CamExport std::list<TopoDS_Face> SubtractFaces( const TopoDS_Face lhs, const TopoDS_Face rhs );
	CamExport Faces_t UnionFaces( const TopoDS_Face lhs, const TopoDS_Face rhs );
	CamExport Faces_t UnionFaces( Faces_t faces );
	CamExport bool FacesIntersect( const TopoDS_Face lhs, const TopoDS_Face rhs );

	CamExport TopoDS_Edge Edge( const Point start, const Point end, const gp_Circ circle );
	CamExport TopoDS_Edge Edge( const Point start, const Point end );

	CamExport TopoDS_Edge Edge( const gp_Pnt start, const gp_Pnt end, const gp_Circ circle );
	CamExport TopoDS_Edge Edge( const gp_Pnt start, const gp_Pnt end );
	CamExport TopoDS_Edge Edge( const TopoDS_Edge original_edge, const Standard_Real start_u, const Standard_Real end_u );

	CamExport double AngleBetweenVectors(
			const gp_Pnt & vector_1_start_point,
			const gp_Pnt & vector_1_end_point,
			const gp_Pnt & vector_2_start_point,
			const gp_Pnt & vector_2_end_point,
			const double minimum_angle );

	CamExport double AngleBetweenVectors(
			const gp_Vec & vector_1,
			const gp_Vec & vector_2,
			const gp_Vec & reference,
			const double minimum_angle );

	class Paths;	// Forward declaration.

	/**
		Container for a single TopoDS_Edge along with all its various parameters.
		NOTE: The OpenCascade library uses the terms FirstParameter() and LastParameter()
		to define a value that is representative of the first end of the edge and the last
		end of the edge.  This class uses the terms StartParameter() and StartPoint() etc. to
		indicate the starting point in terms of movement along the edge.  This distinction
		is the based on the m_is_forwards boolean.  We may want to 'reverse' this edge
		in terms of tool path.  This is simply a reversal of the m_is_forwards boolean
		which allows us to select either the FirstParameter or the LastParameter as the
		StartParameter as appropriate.  I know the terminology is a little confusing but
		part of it comes from OpenCascade and the other part just makes sense in terms
		of tool path progression.
	 */
	class CamExport Path
	{
	public:
		Path(const TopoDS_Edge edge);
		Path & operator= ( const Path & rhs );
		Path( const Path & rhs );

		TopoDS_Edge Edge() const;

		Standard_Real StartParameter() const;
		Standard_Real EndParameter() const;

		Standard_Real Proportion( const double proportion ) const;

		Cam::Point PointAt(const Standard_Real U) const;
		Cam::Point PointAtDistance(const Standard_Real distance) const;
		void D0( const Standard_Real U, gp_Pnt &P) const;
		void D1( const Standard_Real U, gp_Pnt &P, gp_Vec &V ) const;

		Cam::Point StartPoint() const;
		Cam::Point MidPoint() const;
		Cam::Point EndPoint() const;

		gp_Vec StartVector() const;
		gp_Vec EndVector() const;

		Standard_Real Length() const;
		bool IsForwards() const;
		void Reverse();

		std::set<Cam::Point> Intersect( const Path & rhs, const bool stop_after_first_point = false ) const;
		
		Standard_Real Tolerance() const;

		Standard_Boolean operator==( const Path & rhs ) const;
		std::pair<gp_Pnt, gp_Vec> Nearest(const gp_Pnt reference_location, Standard_Real *pParameter = NULL, const bool snap_to_nearest_curve_endpoint = true ) const;
		#ifdef HEEKSCNC
			void Align( CFixture fixture );
		#endif // HEEKSCNC

		Path Section( const Standard_Real start_distance, const Standard_Real end_distance ) const;
		BRepAdaptor_Curve & Curve() const;

		// libArea interface
		std::list<area::CVertex> Vertices() const;
		area::CCurve AreaCurve() const;

		void Transform( const gp_Trsf transformation );
		void Split( const Cam::Paths &areas, Cam::Paths *pInside, Cam::Paths *pOutside ) const;

		QString KiCadBoardOutline(const int layer, const int trace_width) const;

	private:
		TopoDS_Edge m_edge;
		mutable std::auto_ptr<BRepAdaptor_Curve> m_pCurve;
		bool		m_is_forwards;
		mutable Standard_Real m_length;
		mutable Standard_Real m_tolerance;

	public:
		friend QString & operator<< ( QString & str, const Path & path );
	}; // End Path class definition

	/**
		This is a container of multiple Path objects that are both ordered and contiguous (connected)
		It is similar to the Paths (plural) class except that the Paths class can hold a list of
		ContiguousPath objects.  It can also merge ContiguousPath objects if the addition of an edge
		marries two ContiguousPath objects together.
	 */
	class CamExport ContiguousPath
	{
	private:
		typedef enum {
			YZ = 1,
			XZ,
			XY
		} ePlane_t;

	public:
		ContiguousPath();
		ContiguousPath( const ContiguousPath & rhs );
		ContiguousPath & operator= ( const ContiguousPath & rhs );

		bool Add(Path path);
		bool Contains(Path path) const;
		Standard_Real Length() const;
		void D0(const Standard_Real DistanceAlong, gp_Pnt &P);
		void D1(const Standard_Real DistanceAlong, gp_Pnt &P, gp_Vec &V);
		bool IsForwards() const { return(m_is_forwards); }
		void Reverse();
        bool Periodic() const;
		std::vector<Path>::iterator Next(std::vector<Path>::iterator itPath);
		Cam::Point StartPoint() const;
		Cam::Point EndPoint() const;
		bool Surrounds( const Cam::Point point ) const;
		bool Surrounds( const ContiguousPath & rhs ) const;
		TopoDS_Wire Wire() const;
		std::pair<gp_Pnt, gp_Vec> Nearest(const gp_Pnt reference_location) const;
		gp_Pln Plane() const;
		ContiguousPath Section( const Standard_Real start_distance, const Standard_Real end_distance );
		ContiguousPath InterpolateLines(const double max_deviation, const bool retain_simple_curve_types = false) const;
		gp_Pnt Centroid() const;
		std::set<Cam::Point> Intersect( const ContiguousPath & rhs, const bool stop_after_first_point = false ) const;

		ContiguousPath & operator+=( ContiguousPath &rhs );

		std::vector<Path>::iterator SetStartPoint( Cam::Point location );

		// HeeksObj *Sketch(HeeksObj *sketch = NULL);
		void Reset();

		Bnd_Box BoundingBox() const;
		bool IsClockwise();
		bool operator< (const ContiguousPath & rhs ) const;	// Sort by nesting (i.e. object inside object etc.)

		int Concentricity() const;
		void Concentricity(const int value);

		typedef unsigned int Id_t;
		typedef std::set< Id_t > Ids_t;

		Id_t	ID() const;
		Ids_t PathsThatSurroundUs() const;
		void PathsThatSurroundUs(const Id_t id) const;

		Ids_t PathsThatWeSurround() const;
		void PathsThatWeSurround( const Id_t id ) const;
		Cam::Point MidpointForSurroundsCheck() const;

		void Transform( const gp_Trsf transformation );

		#ifdef HEEKSCNC
			void Align( CFixture fixture );
		#endif // HEEKSCNC

		std::vector<Path> Paths();
		void Split( const Cam::Paths &areas, Cam::Paths *pInside, Cam::Paths *pOutside ) const;

		area::CCurve AreaCurve();

		QString KiCadBoardOutline(const int layer, const int trace_width) const;

	protected:
		bool m_is_forwards;
		std::vector<Path> m_paths;
		
		int m_concentricity;	// more positive the value indicates an inner (enclosed) polygon.
		Id_t	m_id;
		static Id_t s_next_available_id;
		mutable Ids_t		m_paths_that_we_surround;
		mutable Ids_t		m_paths_that_surround_us;
		mutable std::auto_ptr<gp_Pln>	m_plane;
		mutable std::auto_ptr<Bnd_Box> m_pBoundingBox;

	public:
		friend QString & operator<< ( QString & str, const ContiguousPath & path );
	}; // End of ContiguousPath class definition.

	class CamExport Paths
	{
	public:
		Paths();
		Paths(const TopoDS_Shape shape);
		Paths(ContiguousPath contiguous_path);
		Paths(area::CArea &area);
		Paths( const Paths & rhs );
		Paths & operator= ( const Paths & rhs );
		Paths & operator-= ( const Paths & rhs );

		void Reverse();

		::size_t size() const;
		ContiguousPath operator[]( const int offset ) const;
		ContiguousPath GetByID( const ContiguousPath::Id_t id ) const;

		Path Next() const;
		void Add(const Path path);
		void Add(const Part::Feature *pFeature);
		void Add(const QStringList input_geometry);
		void Add(const TopoDS_Shape shape);
		void Add(const TopoDS_Edge edge);
		void Add(const TopoDS_Vertex vertex);
		void Add(const TopoDS_Wire wire, const gp_Pln reference_plane, const double maximum_distance);
		void Add(area::CArea &area);
		bool Join( std::vector<ContiguousPath>::iterator lhs, std::vector<ContiguousPath>::iterator rhs );
		bool Offset(const double distance);
		#ifdef HEEKSCNC
			void Align( CFixture fixture );
		#endif // HEEKSCNC

		void Sort(const bool force = false);

		area::CArea Area();
		void Transform( const gp_Trsf transformation );
		Bnd_Box BoundingBox() const;

		void Split( const Cam::Paths &areas, Cam::Paths *pInside, Cam::Paths *pOutside ) const;
		std::set<int> GetConcentricities() const;
		QString KiCadBoardOutline(const int layer = 28, const int trace_width = 150) const;

		Faces_t Faces(const bool subtract_nested_faces = true ) const;

		typedef std::vector<Point> Locations_t;
		Locations_t PointLocationData(const Point reference_location_for_sorting = Point(0.0, 0.0, 0.0)) const;

	protected:
		std::vector<ContiguousPath> m_contiguous_paths;
		std::vector<TopoDS_Vertex>	m_vertices;	// for point data - separate from the contiguous path objects
		bool		m_is_forwards;

		mutable std::auto_ptr<Locations_t> m_pPointLocationData;	// cache to save re-calculation when possible.
	}; // End Paths class definition.


	double deflection( const Cam::Point start, const Cam::Point middle, const Cam::Point end );
	std::list<Cam::Point> InterpolateCurve( BRepAdaptor_Curve curve, const Standard_Real start_u, const Standard_Real end_u, const double max_deviation );



} // End Cam namespace.


