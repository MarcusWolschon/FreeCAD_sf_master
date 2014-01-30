
#include <PreCompiled.h>

#include "Paths.h"
#include "CDouble.h"

#include <App/Application.h>
#include <App/Document.h>
#include <Mod/Part/App/PartFeature.h>
#include <Mod/Part/App/PrimitiveFeature.h>

#include <exception>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgo_Fuse.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepTools.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBndLib.hxx>
#include <Geom_Plane.hxx>
#include <Handle_Geom_Plane.hxx>

#include <BRepOffsetAPI_MakeOffset.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <gp_Circ.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepMesh.hxx>
#include <Poly_Polygon3D.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Handle_BRepAdaptor_HCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Adaptor3d_HCurve.hxx>
#include <Adaptor3d_Curve.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <BRepOffsetAPI_NormalProjection.hxx>
#include <BRepProj_Projection.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <IntTools_EdgeEdge.hxx>
#include <BRepBndLib.hxx>
#include <IntTools_CommonPrt.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <BRepAdaptor_HCurve.hxx>
#include <BRepExtrema_ExtCC.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepLib_MakeSolid.hxx>
#include <TopExp_Explorer.hxx>
#include <gp_Ax1.hxx>
#include <Extrema_ExtPC.hxx>
#include <Geom_OffsetCurve.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <IntTools_SequenceOfPntOn2Faces.hxx>
#include <IntTools_FaceFace.hxx>
#include <Handle_ShapeFix_Wireframe.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <ShapeFix_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <IntTools_BeanBeanIntersector.hxx>

namespace Cam
{
	/* static */ ContiguousPath::Id_t ContiguousPath::s_next_available_id = 1;
	double s_tolerance;
	double Point::s_tolerance;

	double GetTolerance()
	{
		return(0.00001);	// TODO Setup tolerances properly
	}

	TopoDS_Edge Edge( const Point start, const Point end, const gp_Circ circle )
	{
		return(Edge( start.Location(), end.Location(), circle ));
	}

	TopoDS_Edge Edge( const gp_Pnt start, const gp_Pnt end, const gp_Circ circle )
	{
		double tolerance = Cam::GetTolerance();
		const double max_tolerance = 0.01;

		while ((!(start.IsEqual(end, tolerance))) && (tolerance < max_tolerance))
		{
			BRep_Builder aBuilder;
			TopoDS_Vertex vStart, vEnd;

			aBuilder.MakeVertex (vStart, start, tolerance);
			vStart.Orientation (TopAbs_FORWARD);

			aBuilder.MakeVertex (vEnd, end, tolerance);
			vEnd.Orientation (TopAbs_REVERSED);

			BRepBuilderAPI_MakeEdge edge(circle, vStart, vEnd);
			if (! edge.IsDone())
			{
				tolerance *= 10.0;
			}
			else
			{
				return(edge.Edge());
			}
		}

		TopoDS_Edge empty;
		return(empty);

	}

	TopoDS_Edge Edge( const Point start, const Point end )
	{
		return(Edge( start.Location(), end.Location() ));
	}

	TopoDS_Edge Edge( const gp_Pnt start, const gp_Pnt end )
	{
		double tolerance = Cam::GetTolerance();
		const double max_tolerance = 0.01;

		while ((!(start.IsEqual(end, tolerance))) && (tolerance < max_tolerance))
		{
			BRep_Builder aBuilder;
			TopoDS_Vertex vStart, vEnd;

			aBuilder.MakeVertex (vStart, start, tolerance);
			vStart.Orientation (TopAbs_FORWARD);

			aBuilder.MakeVertex (vEnd, end, tolerance);
			vEnd.Orientation (TopAbs_REVERSED);

			BRepBuilderAPI_MakeEdge edge(vStart, vEnd);
			if (! edge.IsDone())
			{
				tolerance *= 10.0;
			}
			else
			{
				return(edge.Edge());
			}
		}

		TopoDS_Edge empty;
		return(empty);
	}


	double AngleBetweenVectors(
			const gp_Pnt & vector_1_start_point,
			const gp_Pnt & vector_1_end_point,
			const gp_Pnt & vector_2_start_point,
			const gp_Pnt & vector_2_end_point,
			const double minimum_angle )
	{
		gp_Vec vector1( vector_1_start_point, vector_1_end_point );
		gp_Vec vector2( vector_2_start_point, vector_2_end_point );
		gp_Vec reference( 0, 0, 1 );    // Looking from the top down.

		double angle = vector1.AngleWithRef( vector2, reference );
		while (angle < minimum_angle) angle += (2 * M_PI);
		return(angle);
	}

	double AngleBetweenVectors(
			const gp_Vec & vector1,
			const gp_Vec & vector2,
			const gp_Vec & reference,	// eg: gp_Vec(0,0,1) to look from the top down
			const double minimum_angle )
	{
		double angle = vector1.AngleWithRef( vector2, reference );
		while (angle < minimum_angle) angle += (2 * M_PI);
		return(angle);
	}

	Point::Point(const gp_Pnt p)
	{
		SetLocation(p);
		SetXDirection(gp_Dir(1.0, 0.0, 0.0));
		SetYDirection(gp_Dir(0.0, 1.0, 0.0));
	}

	Point::Point(const double x, const double y, const double z)
	{
		SetLocation(gp_Pnt(x,y,z));
		SetXDirection(gp_Dir(1.0, 0.0, 0.0));
		SetYDirection(gp_Dir(0.0, 1.0, 0.0));
	}

	Point::Point(const gp_Ax2 ax2) : gp_Ax2(ax2) { }
	Point::Point(const Point & rhs) : gp_Ax2(rhs) { }
	Point & Point::operator= ( const Point & rhs ) { if (this != &rhs) { gp_Ax2::operator=( rhs ); } return(*this); }
	gp_Pnt Point::Location() const { return(gp_Ax2::Location()); }

	Point::Point(const gp_Pln plane, const double x, const double y, const double z)
	{
		SetAxis(plane.Axis());
		SetLocation(gp_Pnt(x,y,z));
	}

	Point::Point(const gp_Pln plane)
	{
		SetAxis(plane.Axis());
		SetLocation(gp_Pnt(0.0,0.0,0.0));
	}


	Point & Point::Location( const gp_Pnt p )
	{
		SetLocation(p);
		return(*this);
	}

	Point & Point::Adjustment( const double x_offset, const double y_offset, const double z_offset )
	{
		return( *this += gp_XYZ( x_offset, y_offset, z_offset ) );
	}

	Point & Point::operator+= ( const gp_XYZ offset )
	{
		XAdjustment(offset.X());
		YAdjustment(offset.Y());
		ZAdjustment(offset.Z());
		return(*this);
	}

	Point & Point::Adjustment( const gp_Dir direction, const double distance )
	{
		gp_Pnt location(gp_Ax2::Location());
		location = gp_Pnt(location.XYZ() + (direction.XYZ() * distance));
		SetLocation(location);
		return(*this);
	}

	Point & Point::XAdjustment( const double offset )
	{
		gp_Pnt location(gp_Ax2::Location());
		location = gp_Pnt(location.XYZ() + (XDirection().XYZ() * offset));
		SetLocation(location);
		return(*this);
	}

	Point & Point::YAdjustment( const double offset )
	{
		gp_Pnt location(gp_Ax2::Location());
		location = gp_Pnt(location.XYZ() + (YDirection().XYZ() * offset));
		SetLocation(location);
		return(*this);
	}

	Point & Point::ZAdjustment( const double offset )
	{
		// gp_Dir direction = XDirection() ^ YDirection();
		gp_Pnt location(gp_Ax2::Location());
		location = gp_Pnt(location.XYZ() + (Direction().XYZ() * offset));
		SetLocation(location);
		return(*this);
	}

	Point & Point::SetX( const double value )
	{
		gp_Pnt location(gp_Ax2::Location());
		location.SetX(value);
		SetLocation(location);
		return(*this);
	}

	Point & Point::SetY( const double value )
	{
		gp_Pnt location(gp_Ax2::Location());
		location.SetY(value);
		SetLocation(location);
		return(*this);
	}

	Point & Point::SetZ( const double value )
	{
		// gp_Dir direction = XDirection() ^ YDirection();
		gp_Pnt location(gp_Ax2::Location());
		location.SetZ(value);
		SetLocation(location);
		return(*this);
	}


	double Point::X(const bool in_drawing_units /* = false */) const
	{
		if (in_drawing_units == false) return(gp_Ax2::Location().X());
		else return(gp_Ax2::Location().X() / Units());
	}
	double Point::Y(const bool in_drawing_units /* = false */) const
	{
		if (in_drawing_units == false) return(gp_Ax2::Location().Y());
		else return(gp_Ax2::Location().Y() / Units());
	}
	double Point::Z(const bool in_drawing_units /* = false */) const
	{
		if (in_drawing_units == false) return(gp_Ax2::Location().Z());
		else return(gp_Ax2::Location().Z() / Units());
	}

	double Point::Units() const
	{
		return(1.0);	// TODO Handle metric or imperial units (25.4 for inches)
	}

	double Point::Tolerance() const
	{
		if (s_tolerance <= 0.0)
		{
			s_tolerance = Cam::GetTolerance();
		}
		return(s_tolerance);
	}

	bool Point::operator==( const Point & rhs ) const
	{
		// We use the sum of both point's tolerance values.
		return(gp_Ax2::Location().Distance(rhs.Location()) <= (Tolerance() + rhs.Tolerance()));
	} // End equivalence operator

	bool Point::operator!=( const Point & rhs ) const
	{
		return(! (*this == rhs));
	} // End not-equal operator

	bool Point::operator<( const Point & rhs ) const
	{
		if (*this == rhs) return(false);

		if (fabs(X() - rhs.X()) > (Tolerance() + rhs.Tolerance()))
		{
			if (X() > rhs.X()) return(false);
			if (X() < rhs.X()) return(true);
		}

		if (fabs(Y() - rhs.Y()) > (Tolerance() + rhs.Tolerance()))
		{
			if (Y() > rhs.Y()) return(false);
			if (Y() < rhs.Y()) return(true);
		}

		if (fabs(Z() - rhs.Z()) > (Tolerance() + rhs.Tolerance()))
		{
			if (Z() > rhs.Z()) return(false);
			if (Z() < rhs.Z()) return(true);
		}

		return(false);	// They're equal
	} // End equivalence operator

	void Point::ToDoubleArray( double *pArrayOfThree ) const
	{
		pArrayOfThree[0] = X();
		pArrayOfThree[1] = Y();
		pArrayOfThree[2] = Z();
	} // End ToDoubleArray() method


	double Point::XYDistance( const Point & rhs ) const
	{
		gp_Pnt _lhs(this->Location());
		gp_Pnt _rhs(rhs.Location());
		_lhs.SetZ(0.0);
		_rhs.SetZ(0.0);
		return(_lhs.Distance(_rhs));
	}

	double Point::XZDistance( const Point & rhs ) const
	{
		gp_Pnt _lhs(this->Location());
		gp_Pnt _rhs(rhs.Location());
		_lhs.SetY(0.0);
		_rhs.SetY(0.0);
		return(_lhs.Distance(_rhs));
	}

	double Point::YZDistance( const Point & rhs ) const
	{
		gp_Pnt _lhs(this->Location());
		gp_Pnt _rhs(rhs.Location());
		_lhs.SetX(0.0);
		_rhs.SetX(0.0);
		return(_lhs.Distance(_rhs));
	}



	Ax3::Ax3( const gp_Ax3 & rhs ) : gp_Ax3(rhs) {}



	Path::Path(const TopoDS_Edge edge): m_edge(edge), m_is_forwards(true), m_length(-1), m_tolerance(-1) { }

	Path & Path::operator= ( const Path & rhs )
	{
		if (this != &rhs)
		{
			BRepBuilderAPI_Copy duplicate;
			duplicate.Perform(rhs.m_edge);
			m_edge = TopoDS::Edge(duplicate.Shape());
			m_length = rhs.m_length;
			m_is_forwards = rhs.m_is_forwards;
			m_tolerance = rhs.m_tolerance;
			m_pCurve.reset(NULL);
		}

		return(*this);
	}

	Path::Path( const Path & rhs )
	{
		*this = rhs;	// call the assignment operator.
	}

	TopoDS_Edge Path::Edge() const 
	{ 
		BRepBuilderAPI_Copy duplicate;
		duplicate.Perform(m_edge);
		return(TopoDS::Edge(duplicate.Shape())); 
	}

	Cam::Point Path::PointAt(const Standard_Real U) const
	{
		gp_Pnt point;
		Curve().D0(U,point);
		return(point);
	}

	Cam::Point Path::PointAtDistance(const Standard_Real distance) const
	{
		Standard_Real U = Proportion(distance/Length());
		return(PointAt(U));
	}

	void Path::D0( const Standard_Real U, gp_Pnt &P) const
	{
		Curve().D0(U,P);
	}

	void Path::D1( const Standard_Real U, gp_Pnt &P, gp_Vec &V ) const
	{
		Curve().D1(U,P,V);
	}

	Cam::Point Path::StartPoint() const
	{
		gp_Pnt point;
		Curve().D0(StartParameter(),point);
		return(point);
	}

	Cam::Point Path::MidPoint() const
	{
		gp_Pnt point;
		Curve().D0(Proportion(0.5),point);
		return(point);
	}

	Cam::Point Path::EndPoint() const
	{
		gp_Pnt point;
		Curve().D0(EndParameter(),point);
		return(point);
	}

	gp_Vec Path::StartVector() const
	{
		gp_Vec vec;
		gp_Pnt point;
		Curve().D1(StartParameter(), point, vec);
		if (! m_is_forwards) vec.Reverse();
		return(vec);
	}

	gp_Vec Path::EndVector() const
	{
		gp_Vec vec;
		gp_Pnt point;
		Curve().D1(EndParameter(), point, vec);
		if (! m_is_forwards) vec.Reverse();
		return(vec);
	}

	Standard_Real Path::Length() const
	{
		if (m_length < 0)
		{
			m_length = GCPnts_AbscissaPoint::Length( Curve() );
		}

		return(m_length);
    }

	bool Path::IsForwards() const { return(m_is_forwards); }

	void Path::Reverse()
	{
		m_is_forwards = ! m_is_forwards;
	}

	Standard_Real Path::Tolerance() const
	{
		if (m_tolerance < 0)
		{
			m_tolerance = Cam::GetTolerance();
		}

		return(m_tolerance); 
	}

	BRepAdaptor_Curve & Path::Curve() const
	{
		if (m_pCurve.get() == NULL)
		{
			m_pCurve = std::auto_ptr<BRepAdaptor_Curve>(new BRepAdaptor_Curve(m_edge));
		}

		return(*m_pCurve); 
	}


	QString & operator<< ( QString & str, const Path & path )
	{
		str.append(QString::fromUtf8("<Path "));

		switch(path.Curve().GetType())
		{
		case GeomAbs_Line:
			str.append(QString::fromUtf8("type=\"GeomAbs_Line\""));
			break;

		case GeomAbs_Circle:
			str.append(QString::fromUtf8("type=\"GeomAbs_Circle\""));
			break;

		case GeomAbs_Ellipse:
			str += QString::fromUtf8("type=\"GeomAbs_Ellipse\"");
			break;

		case GeomAbs_Hyperbola:
			str += QString::fromUtf8("type=\"GeomAbs_Hyperbola\"");
			break;

		case GeomAbs_Parabola:
			str += QString::fromUtf8("type=\"GeomAbs_Parabola\"");
			break;

		case GeomAbs_BezierCurve:
			str += QString::fromUtf8("type=\"GeomAbs_BezierCurve\"");
			break;

		case GeomAbs_BSplineCurve:
			str += QString::fromUtf8("type=\"GeomAbs_BSplineCurve\"");
			break;

		case GeomAbs_OtherCurve:
			str += QString::fromUtf8("type=\"GeomAbs_OtherCurve\"");
			break;
		}

		str += QString::fromUtf8(", direction=\"") + QString(path.m_is_forwards?QString::fromUtf8("FORWARDS"):QString::fromUtf8("BACKWARDS")) + QString::fromUtf8("\"");
		str += QString::fromUtf8(", start_parameter=\"") + QString().arg(path.StartParameter()) + QString::fromUtf8("\"");
		str += QString::fromUtf8(", end_parameter=\"") + QString().arg(path.EndParameter()) + QString::fromUtf8("\"");

		str += QString::fromUtf8(", start_point=\"") + QString().arg(path.StartPoint().X()) + QString::fromUtf8(",") + QString().arg(path.StartPoint().Y()) + QString::fromUtf8(",") + QString().arg(path.StartPoint().Z()) + QString::fromUtf8("\"");
		str += QString::fromUtf8(", end_point=\"") + QString().arg(path.EndPoint().X()) + QString::fromUtf8(",") + QString().arg(path.EndPoint().Y()) + QString::fromUtf8(",") + QString().arg(path.EndPoint().Z()) + QString::fromUtf8("\"/>\n");
		return(str);
	}










	std::list<TopoDS_Face> IntersectFaces( const TopoDS_Face lhs, const TopoDS_Face rhs )
	{
		std::list<TopoDS_Face> results;

		TopoDS_Shape shape;

		try {
			BRepAlgoAPI_Common common( lhs, rhs );
			common.Build();
			if (common.IsDone())
			{
				shape = common.Shape();

				for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
				{
					TopoDS_Face aFace = TopoDS::Face(expFace.Current());
					results.push_back(aFace);
				}
			} // End if - then
			return(results);
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			return(results);
		}
	}


	std::list<TopoDS_Face> SubtractFaces( const TopoDS_Face lhs, const TopoDS_Face rhs )
	{
		std::list<TopoDS_Face> results;

		TopoDS_Shape shape;

		try {
			BRepAlgoAPI_Cut cut( lhs, rhs );
			cut.Build();
			if (cut.IsDone())
			{
				shape = cut.Shape();

				for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
				{
					TopoDS_Face aFace = TopoDS::Face(expFace.Current());
					results.push_back(aFace);
				}
			} // End if - then
			return(results);
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			return(results);
		}
	}

	bool FacesIntersect( const TopoDS_Face lhs, const TopoDS_Face rhs )
	{
		try
		{
			if (lhs.IsNull()) return(false);
			if (rhs.IsNull()) return(false);

			Bnd_Box lhs_bounding_box, rhs_bounding_box;
			BRepBndLib::Add(lhs, lhs_bounding_box);
			BRepBndLib::Add(rhs, rhs_bounding_box);

			if (lhs_bounding_box.IsOut(rhs_bounding_box)) return(false);

			IntTools_FaceFace intersect;
			intersect.Perform( lhs, rhs );
			if (intersect.IsDone() == Standard_False)
			{
				return(false);
			}

			for (TopExp_Explorer lhsExplorer(lhs, TopAbs_EDGE); lhsExplorer.More(); lhsExplorer.Next())
			{
				TopoDS_Edge edge1 = TopoDS::Edge(lhsExplorer.Current());

				if (! edge1.IsNull())
				{
					for (TopExp_Explorer rhsExplorer(rhs, TopAbs_EDGE); rhsExplorer.More(); rhsExplorer.Next())
					{
						TopoDS_Edge edge2 = TopoDS::Edge(rhsExplorer.Current());

						if (! edge2.IsNull())
						{
							IntTools_BeanBeanIntersector intersector(edge1, edge2);
							intersector.Perform();
							if ((intersector.IsDone()) && (intersector.Result().Length() > 0))
							{
								return(true);
							}
						} // End if - then
					} // End for
				} // End if - then
			} // End for

			// They do overlap but they do NOT have intersection points.
			return(false);
		}
		catch( Standard_Failure )
		{
			return(false);
		}
	}


	Standard_Real Area(const TopoDS_Shape shape)
	{
		GProp_GProps System;
		BRepGProp::SurfaceProperties(shape,System);
		return System.Mass();
	}
	


	Faces_t UnionFaces( Faces_t faces )
	{
		#ifdef HEEKSCAD
			// return(RS274X::UnionFaces(faces));
		#endif

		// We have a list of faces that represent all the copper areas we want.  We now need to intersect
		// all the touching faces to produce a single (larger) face.  The ones that intersect each other
		// should be replaced by the larger (combined) face.  We should repeat this until we find no
		// further intersecting faces.

		Faces_t union_of_faces;

		while (faces.size() > 1)
		{
			// Join all other faces to the first one.  Keep joining until we can't find any to join.
			bool joins_made = false;
			std::list<Faces_t::iterator> already_joined;
			Faces_t::iterator itFace = faces.begin(); itFace++;
			for ( ; (! joins_made) && (itFace != faces.end()); itFace++)
			{
				if (FacesIntersect( *(faces.begin()), *itFace ))
				{
					Faces_t combinations = UnionFaces(*(faces.begin()), *itFace);
					if (combinations.size() > 0)
					{
						*(faces.begin()) = *(combinations.begin());
						joins_made = true;
						already_joined.push_back(itFace);
					}
				} // End if - then
			} // End for

			for (std::list<Faces_t::iterator>::iterator itRemove = already_joined.begin(); itRemove != already_joined.end(); itRemove++)
			{
				faces.erase( *itRemove );
			}

			if (joins_made == false)
			{
				// This is as big as it's ever going to get.
				TopoDS_Face face(*(faces.begin()));
				if (Area(face) > 0.0001)
				{
					union_of_faces.push_back( face );
				}

				faces.erase( faces.begin() );
			}
		} // End while

		if (faces.size() > 0)
		{
			TopoDS_Face face(*(faces.begin()));

			if (Area(face) > 0.0001)
			{
				union_of_faces.push_back( face );
			}
			faces.erase( faces.begin() );
		}

		return(union_of_faces);

	} // End UnionFaces() method

	

	Faces_t UnionFaces( const TopoDS_Face lhs, const TopoDS_Face rhs )
	{
		Faces_t faces;

		if (lhs.IsNull() && ! rhs.IsNull())
		{
			faces.push_back(rhs);
			return(faces);
		}
		else if (! lhs.IsNull() && rhs.IsNull())
		{
			faces.push_back(lhs);
			return(faces);
		}
		else if (lhs.IsNull() && rhs.IsNull())
		{
			return(faces); // empty.
		}

		if ((Area(lhs) < 0.00001) || (Area(rhs) < 0.00001))
		{
			return(faces); // empty.
		}

		{
			Paths confirm_closed(lhs);
			if ((confirm_closed.size() != 1)  || (! confirm_closed[0].Periodic())) return(faces);
		}

		{
			Paths confirm_closed(rhs);
			if ((confirm_closed.size() != 1)  || (! confirm_closed[0].Periodic())) return(faces);
		}

		/*
		#ifdef HEEKSCAD
			heekscad_interface.Add( heekscad_interface.ConvertShapeToSketch(lhs, 0.01), NULL );
			heekscad_interface.Add( heekscad_interface.ConvertShapeToSketch(rhs, 0.01), NULL );
		#endif // HEEKSCAD
		*/

		try {
			BRepAlgo_Fuse fuse( lhs, rhs );
			fuse.Build();
			if (fuse.IsDone())
			{
				TopoDS_Shape shape = fuse.Shape();

				for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
				{
					/*
					TopoDS_Face aFace = TopoDS::Face(expFace.Current());
					faces.push_back(aFace);
					*/
					
					TopoDS_Face aFace = TopoDS::Face(expFace.Current());
					TopoDS_Wire wire=BRepTools::OuterWire(aFace);
					ShapeFix_Wire fix;
					fix.Load( wire );
					fix.FixReorder();
					fix.FixConnected();
					fix.FixClosed();

					Paths confirm_closed(fix.Wire());
					if ((confirm_closed.size() == 1)  && (confirm_closed[0].Periodic()))
					{
						faces.push_back(BRepBuilderAPI_MakeFace(fix.Wire()));
					}
				}
			} // End if - then

			return(faces);
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			Faces_t empty;
			return(empty);
		}
	}


/**
	Return the U value that is this proportion of the way between the StartParameter()
	and the EndParameter().  The U value is some floating point value that represents
	how far along the Path one wants.  For a line, it's a distance but for an arc it's
	the number of radians.  We can't infer anything from the value but we can use it
	to move a proportion of the way along a Path object, no matter what type of object
	it is.
 */
Standard_Real Cam::Path::Proportion( const double proportion ) const
{
    if (StartParameter() < EndParameter())
    {
        return(((EndParameter() - StartParameter()) * proportion) + StartParameter());
    }
    else
    {
        return(((StartParameter() - EndParameter()) * proportion) + EndParameter());
    }
}


Standard_Boolean Cam::Path::operator==( const Path & rhs ) const
{
	if (m_edge.ShapeType() != rhs.m_edge.ShapeType()) return(false);
	if (fabs(Length() - rhs.Length()) > Cam::GetTolerance()) return(false);

	std::set<Cam::Point> lhsPoints, rhsPoints;
	lhsPoints.insert( StartPoint() );
	lhsPoints.insert( MidPoint() );
	lhsPoints.insert( EndPoint() );

	rhsPoints.insert( rhs.StartPoint() );
	rhsPoints.insert( rhs.MidPoint() );
	rhsPoints.insert( rhs.EndPoint() );

	if (lhsPoints != rhsPoints) return(false);

	return(true);
}


ContiguousPath::ContiguousPath()
{ 
	m_is_forwards = true; m_concentricity = -1; m_id = s_next_available_id++; 
}


int ContiguousPath::Concentricity() const { return(m_concentricity); }
void ContiguousPath::Concentricity(const int value) { m_concentricity = value; }

ContiguousPath::Id_t	ContiguousPath::ID() const { return(m_id); }
ContiguousPath::Ids_t ContiguousPath::PathsThatSurroundUs() const { return(m_paths_that_surround_us); }


ContiguousPath::Ids_t ContiguousPath::PathsThatWeSurround() const { return(m_paths_that_we_surround); }


QString & operator<< ( QString & str, const ContiguousPath & path )
{
	str += QString::fromUtf8("<ContiguousPath ") + QString::fromUtf8(" num_paths=\"") + QString().arg(path.m_paths.size()) + QString::fromUtf8("\"");
	str += QString::fromUtf8(", direction=\"") + QString(path.m_is_forwards?QString::fromUtf8("FORWARDS"):QString::fromUtf8("BACKWARDS")) + QString::fromUtf8("\"") + QString::fromUtf8(">\n");

	for (std::vector<Path>::const_iterator itPath = path.m_paths.begin(); itPath != path.m_paths.end(); itPath++)
	{
		str << *itPath;
	}

	str += QString::fromUtf8("</ContiguousPath>\n");

	return(str);
}


Paths::Paths() { }


void Paths::Reverse()
{
	m_is_forwards = ! m_is_forwards;
	for (std::vector<ContiguousPath>::iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		itPath->Reverse();
	}
}

::size_t Paths::size() const { return(m_contiguous_paths.size()); }
ContiguousPath Paths::operator[]( const int offset ) const { return(m_contiguous_paths[offset]); }



/**
	Adjust the vector of path objects so that the start point matches the X,Y values
	of the location specified.  If this is half way through the vector of path objects
	then the vector needs to be rearranged so that it's direction is still correct
	but the first item in the vector matches this location.  We normally do this so that
	the outline starts machining from the CNC machine's current location.
 */
std::vector<Cam::Path>::iterator Cam::ContiguousPath::SetStartPoint( Cam::Point location )
{
    double tolerance = Cam::GetTolerance();

	// Find the nearest coordinate to 'location'
	Cam::Point nearest(m_paths.begin()->StartPoint());
	for (std::vector<Path>::iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		CDouble distance_to_location = itPath->StartPoint().XYDistance(location);
		CDouble distance_to_nearest = itPath->StartPoint().XYDistance(nearest);

		if (distance_to_location < distance_to_nearest)
        {
			nearest = itPath->StartPoint();
		}
	 }

    // This will only work if the path is periodic (i.e. a closed shape)
    // if (m_paths.begin()->StartPoint() != m_paths.rbegin()->EndPoint())
	if (! Periodic())
    {
        // We can't rearrange the list. See if we're already in position.
        if (m_paths.begin()->StartPoint().XYDistance(nearest) < tolerance)
        {
            return(m_paths.begin());
        }
        else if (m_paths.rbegin()->EndPoint().XYDistance(nearest) < tolerance)
        {
            Reverse();
            return(m_paths.end()-1);
        }
        else
        {
            // It's not periodic and the location doesn't line up
            // with either of the endpoints.

            return(m_paths.begin());
        }
    }

    // Look for this location in the start/end points of this path.
    for (std::vector<Path>::iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
    {
        if (itPath->StartPoint().XYDistance(nearest) < tolerance)
        {
            return(itPath);
        }
    }
	return(m_paths.begin());	// It must have been periodic but the location was not at any of the starting points.
}


Cam::Paths::Paths(Cam::ContiguousPath contiguous_path)
{
	std::vector<Path> paths = contiguous_path.Paths();
	for (std::vector<Path>::iterator itPath = paths.begin(); itPath != paths.end(); itPath++)
	{
		Add(*itPath);
	}
}

Cam::Paths::Paths(const TopoDS_Shape shape)
{
	Add(shape);
}

/*
Cam::Paths::Paths(HeeksObj *object)
{
	Add(object);
}
*/

Cam::Paths::Paths(area::CArea & area)
{
	Add(area);
}


void Cam::Paths::Add(area::CArea & area)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	for (std::list<area::CCurve>::const_iterator itCurve = area.m_curves.begin(); itCurve != area.m_curves.end(); itCurve++)
	{
		std::list<area::Span> spans;
		itCurve->GetSpans( spans );
		for (std::list<area::Span>::const_iterator itSpan = spans.begin(); itSpan != spans.end(); itSpan++)
		{
			switch (itSpan->m_v.m_type)
			{
			case 0:	// line (or start point)
				Add( Cam::Edge( gp_Pnt(itSpan->m_p.x, itSpan->m_p.y, 0.0), gp_Pnt(itSpan->m_v.m_p.x, itSpan->m_v.m_p.y, 0.0) ) );
				break;

			case -1:	// CW arc
				{
					gp_Pnt start(itSpan->m_p.x, itSpan->m_p.y, 0.0);
					gp_Pnt centre(itSpan->m_v.m_c.x, itSpan->m_v.m_c.y, 0.0);
					gp_Pnt end(itSpan->m_v.m_p.x, itSpan->m_v.m_p.y, 0.0);
					gp_Circ circle;
					circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0, 0.0, -1.0)));
					circle.SetLocation(centre);
					circle.SetRadius(start.Distance(centre));
					Add( Cam::Edge( start, end, circle ));
				}
				break;

			case 1:	// CCW arc
				{
					gp_Pnt start(itSpan->m_p.x, itSpan->m_p.y, 0.0);
					gp_Pnt centre(itSpan->m_v.m_c.x, itSpan->m_v.m_c.y, 0.0);
					gp_Pnt end(itSpan->m_v.m_p.x, itSpan->m_v.m_p.y, 0.0);
					gp_Circ circle;
					circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0, 0.0, 1.0)));
					circle.SetLocation(centre);
					circle.SetRadius(start.Distance(centre));
					Add( Cam::Edge( start, end, circle ));
				}
				break;
			}
		} // End for
	} // End for
}



bool Cam::ContiguousPath::operator< (const Cam::ContiguousPath & rhs ) const
{
	Ids_t ids_we_surround = PathsThatWeSurround();
	Ids_t ids_that_surround_us = rhs.PathsThatSurroundUs();
	if (ids_we_surround.find(rhs.ID()) != ids_we_surround.end()) return(true);	// We'v already figured this out.
	if (ids_that_surround_us.find(ID()) != ids_that_surround_us.end()) return(true);	// We'v already figured this out.

	if (this->Surrounds(rhs))
	{
		PathsThatWeSurround(rhs.ID());
		rhs.PathsThatSurroundUs(ID());

		return(true);
	}

	return(false);
}

gp_Pnt Cam::ContiguousPath::Centroid() const
{
	Standard_Real aXmin;
	Standard_Real aYmin;
	Standard_Real aZmin;
	Standard_Real aXmax;
	Standard_Real aYmax;
	Standard_Real aZmax;

	BoundingBox().Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);

	gp_Pnt centroid(((aXmax - aXmin)/2.0) + aXmin,
					((aYmax - aYmin)/2.0) + aYmin,
					((aZmax - aZmin)/2.0) + aZmin );
	return(centroid);
}


struct sort_paths_by_centroid_distance : public std::binary_function< const ContiguousPath &, const ContiguousPath &, bool >
{
	sort_paths_by_centroid_distance( const ContiguousPath & reference_path )
	{
		m_reference_path = reference_path;
		m_reference_point = reference_path.Centroid();
	} // End constructor

	ContiguousPath m_reference_path;
	gp_Pnt m_reference_point;

	// Return true if dist(lhs to ref) < dist(rhs to ref)
	bool operator()( const ContiguousPath & lhs, const ContiguousPath & rhs ) const
	{
		return( lhs.Centroid().Distance( m_reference_point ) < rhs.Centroid().Distance( m_reference_point ) );
	} // End operator() overload
}; // End sort_paths_by_centroid_distance structure definition.


void Cam::Paths::Sort(const bool force /* = false */)
{

	if (! force)
	{
		// Check to see if all of our contiguous paths already have a concentricity >= 0 (indicating
		// that sorting has already occured)
		for (std::vector<ContiguousPath>::iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
		{
			if (itPath->Concentricity() >= 0) return;	// It has already been done.
		}
	}

	// Initialize all contiguous paths as 'top level' (i.e. concentricity = 0) to start with.  These
	// values will be incremented as nested paths are discovered during the sorting process.
	for (std::vector<ContiguousPath>::iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		// set concentricity = 0 and remove any parent/child ID lists.
		itPath->Reset();
	}

	std::sort(m_contiguous_paths.begin(), m_contiguous_paths.end());

	for (std::vector<ContiguousPath>::iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		for (std::vector<ContiguousPath>::iterator itInnerPath = itPath; itInnerPath != m_contiguous_paths.end(); itInnerPath++)
		{
			if (itInnerPath == itPath) continue;

			if (itPath->Surrounds(*itInnerPath))
			{
				itInnerPath->Concentricity( itPath->Concentricity() + 1);
				itInnerPath->PathsThatWeSurround( itPath->ID() );
				itPath->PathsThatSurroundUs( itInnerPath->ID() );
			}
		}
	}

	// Now go through and sort the list based on its distance from the previous shape.
	for (std::vector<ContiguousPath>::iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		// We've already begun.  Just sort based on the previous point's location.
		std::vector<ContiguousPath>::iterator l_itNextPath = itPath;
		l_itNextPath++;

		if (l_itNextPath != m_contiguous_paths.end())
		{
			sort_paths_by_centroid_distance compare( *itPath );
			std::sort( l_itNextPath, m_contiguous_paths.end(), compare );
		} // End if - then
	} // End for
}


bool Cam::ContiguousPath::Contains(Path path) const
{
	if (m_paths.size() == 0) return(false);
	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		if (*itPath == path) return(true);
	}

	return(false);
}


/**
	Try to add this path to the contiguous list of paths.  If it doesn't
	connect with the existing list of paths then it returns failure.
 */
bool Cam::ContiguousPath::Add(Cam::Path path)
{
	m_plane.reset(NULL);
	m_pBoundingBox.reset(NULL);

	if (m_paths.size() == 0)
	{
		m_concentricity = -1;	// Indicate that our sorting has not been done.
		m_paths.push_back(path);
		return(true);
	}
	else if (m_paths.begin()->StartPoint() == path.StartPoint())
	{
		m_concentricity = -1;	// Indicate that our sorting has not been done.
		path.Reverse();
		m_paths.insert(m_paths.begin(), path);
		return(true);
	}
	else if (m_paths.begin()->StartPoint() == path.EndPoint())
	{
		m_concentricity = -1;	// Indicate that our sorting has not been done.
		m_paths.insert(m_paths.begin(), path);
		return(true);
	}
	else if (m_paths.rbegin()->EndPoint() == path.StartPoint())
	{
		m_concentricity = -1;	// Indicate that our sorting has not been done.
		m_paths.push_back(path);
		return(true);
	}
	else if (m_paths.rbegin()->EndPoint() == path.EndPoint())
	{
		m_concentricity = -1;	// Indicate that our sorting has not been done.
		path.Reverse();
		m_paths.push_back(path);
		return(true);
	}
	else
	{
		return(false);
	}
}


void Cam::Paths::Add(const QStringList input_geometry_names)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	App::Document *document = App::GetApplication().getActiveDocument();
	if (document)
	{
		for (QStringList::size_type i=0; i<input_geometry_names.size(); i++)
		{
			App::DocumentObject *object = document->getObject(input_geometry_names[i].toAscii().constData());
			if(object && object->isDerivedFrom(Part::Feature::getClassTypeId())) 
			{
				Part::Feature *part_feature = dynamic_cast<Part::Feature *>(object);
				if (part_feature)
				{
					Add(part_feature);
				}
			}
		}
	}
}

/**
	This is the conversion from a Cam::Feature into a series of TopoDS_Edge objects.  If
	the TopoDS_Shape returned represents a solid or some other feature that we're not 
	interested in then it will be quietly discarded.
 */
void Cam::Paths::Add( const Part::Feature *link )
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	if (!link) return;
    if (link->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
	{
		const TopoDS_Shape& shape = static_cast<const Part::Feature*>(link)->Shape.getShape()._Shape;
		if (shape.IsNull() == false) 
		{
			Add(shape);
		}
	}
	/*
	else if (link->getTypeId().isDerivedFrom(Sketcher::SketchObject::getClassTypeId()))
	{
		// It must be a vertex contained within a sketch.

		const Sketcher::SketchObject *sketch = dynamic_cast<const Sketcher::SketchObject *>(link);
		if (sketch)
		{
			const std::vector<Part::Geometry *> &getInternalGeometry(void) const { return Geometry.getValues(); }

			BRepBuilderAPI_MakeVertex make_vertex(gp_Pnt(part_vertex->X.getValue(), part_vertex->Y.getValue(), part_vertex->Z.getValue()));
			Add(make_vertex.Vertex());
		}
	}
	*/
}

void Cam::Paths::Add(const TopoDS_Vertex vertex)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.
	m_vertices.push_back(vertex);
}

void Cam::Paths::Add(const TopoDS_Shape shape)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	if (! shape.IsNull())
	{
		TopAbs_ShapeEnum shape_type(shape.ShapeType());

		switch (shape_type)
		{
		case TopAbs_VERTEX:
			Add(TopoDS::Vertex(shape));
			break;

		case TopAbs_EDGE:
			Add(Path(TopoDS::Edge(shape)));
			break;

		case TopAbs_WIRE:
		case TopAbs_FACE:
		case TopAbs_COMPOUND:
		default:
			{
				TopoDS_Iterator It(shape, Standard_True, Standard_True);
				for (; It.More(); It.Next())
				{
					Add(It.Value());
				}
			}
			break;
		}
	}
}

void Paths::Add(const TopoDS_Edge edge)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	if (IsValid(edge))
	{
		Add(Path(edge));
	}
}


void Paths::Add(const TopoDS_Wire wire, const gp_Pln reference_plane, const double maximum_distance)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	CDouble allowable_distance = fabs(maximum_distance);
	if (IsValid(wire))
	{
		for(BRepTools_WireExplorer expEdge(wire); expEdge.More(); expEdge.Next())
		{
			Path path(expEdge.Current());

			if ((CDouble(reference_plane.Distance(path.StartPoint().Location())) <= allowable_distance) &&
				(CDouble(reference_plane.Distance(path.EndPoint().Location())) <= allowable_distance) &&
				(CDouble(reference_plane.Distance(path.MidPoint().Location())) <= allowable_distance))
			{
				Add(path);
			}
		}
	}
}



/**
	Add a single Path (really a TopoDS_Edge) into either an existing ContiguousPath
	object or a new ContiguousPath object.  If the addition of this Path into an
	existing one means that two ContiguousPath objects can now be joined, then join
	them to form a single, longer, ContiguousPath object.  We always want our list
	of ContiguousPath objects to be connected if possible.
 */
void Cam::Paths::Add(const Path path)
{
	m_pPointLocationData.reset(NULL);	// reset the cache to indicate it's out of date.

	bool added = false;

	// See if any of the paths already contain this path.  If so, don't add the duplicate copy.
	for (std::vector<ContiguousPath>::iterator itContiguousPath = m_contiguous_paths.begin(); (! added) && (itContiguousPath != m_contiguous_paths.end()); itContiguousPath++)
	{
		if (itContiguousPath->Contains(path))
		{
			// It already exists here.  Don't add the duplicate.
			return;
		}
	}

	// See if we can add it to any of the existing contiguous paths.
	for (std::vector<ContiguousPath>::iterator itContiguousPath = m_contiguous_paths.begin(); (! added) && (itContiguousPath != m_contiguous_paths.end()); itContiguousPath++)
	{
		if (itContiguousPath->Add(path))
		{
			added = true;
		}
	}

	if (! added)
	{
		ContiguousPath new_path;
		new_path.Add(path);
		m_contiguous_paths.push_back(new_path);
		return;
	}
	else
	{
		// It was added to one of the existing paths.  We need to check to see if this is the
		// missing link joining any of the existing paths together.

		bool join_found = false;
		do {
			join_found = false;

			for (std::vector<ContiguousPath>::iterator lhs = m_contiguous_paths.begin();
				 (join_found == false) && (lhs != m_contiguous_paths.end()); lhs++)
			{
				for (std::vector<ContiguousPath>::iterator rhs = m_contiguous_paths.begin();
				 (join_found == false) && (rhs != m_contiguous_paths.end()); rhs++)
				{
					if (lhs == rhs) continue;

					if (lhs->StartPoint() == rhs->EndPoint())
					{
						join_found = true;
						*rhs += *lhs;
						lhs = m_contiguous_paths.erase(lhs);
						break;
					}
					else if (lhs->EndPoint() == rhs->StartPoint())
					{
						join_found = true;
						*lhs += *rhs;
						rhs = m_contiguous_paths.erase(rhs);
						break;
					}
					else if (lhs->StartPoint() == rhs->StartPoint())
					{
						join_found = true;
						rhs->Reverse();
						*lhs += *rhs;
						rhs = m_contiguous_paths.erase(rhs);
						break;
					}
					else if (lhs->EndPoint() == rhs->EndPoint())
					{
						join_found = true;
						rhs->Reverse();
						*lhs += *rhs;
						rhs = m_contiguous_paths.erase(rhs);
						break;
					}
				} // End for
			} // End for
		} while (join_found);
	}
}


/**
	Append all Path objects within the rhs contiguous path into our own contiguous path.
	We use the Add() routine to add the Path objects at either the beginning or the end of this
	contiguous path as necessary.
 */
Cam::ContiguousPath & Cam::ContiguousPath::operator+=( ContiguousPath &rhs )
{
	for (std::vector<Path>::iterator itPath = rhs.m_paths.begin(); itPath != rhs.m_paths.end(); itPath++)
	{
		Add(*itPath);
	}

	return(*this);
}



/**
	Aggregate the lengths of all Path objects we contain.
 */
Standard_Real Cam::ContiguousPath::Length() const
{
	Standard_Real length = 0.0;
	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		length += itPath->Length();
	}

	return(length);
}

/**
	This routine reverses the direction of each path object as well as sets a boolean
	indicating the direction to use when iterating through the Path objects.  The order
	of the actual list of Path objects is not changed.  Once this is done, the Ramp()
	method is always able to move from StartPoint() to EndPoint() no matter whether it's
	going forwards or backwards through the list of Path objects.
 */
void Cam::ContiguousPath::Reverse()
{
	for (std::vector<Path>::iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		itPath->Reverse();
	}
	m_is_forwards = ! m_is_forwards;
}


bool Cam::ContiguousPath::Periodic() const
{
    if (StartPoint() == EndPoint()) return(true);
    return(false);
}


std::vector<Cam::Path> Cam::ContiguousPath::Paths()
{
	std::set< std::vector<Cam::Path>::iterator > already_added;
	std::vector<Cam::Path> paths;
	std::vector<Cam::Path>::iterator itPath;
	if (this->m_is_forwards)
	{
		itPath = m_paths.begin();
	}
	else
	{
		itPath = m_paths.end();
	}
	do
	{
		paths.push_back(*itPath);
		already_added.insert(itPath);

		itPath = Next(itPath);
	} while (already_added.find(itPath) == already_added.end());

	return(paths);
}

/**
	If we're iterating through the ContiguousPath, this routine figures out what the next
	Path in the chain is.  It handles going forwards and backwards through the list of
	Path objects.  It also handles whether the ContiguousPath is Periodic (i.e. a closed
	shape) or not.  The Ramp() method uses this method to iterate backwards and forwards
	through the Path objects moving slowly downwards in Z until the necessary depths are
	achieved.
 */
std::vector<Cam::Path>::iterator Cam::ContiguousPath::Next(std::vector<Cam::Path>::iterator itPath)
{
    if (m_paths.size() == 1)
    {
        Reverse();
        return(itPath);
    }

    if (m_is_forwards)
    {
        if (itPath == m_paths.end())
        {
            if (Periodic())
            {
                itPath = m_paths.begin();
            }
            else
            {
                Reverse();
            }
        }
        else
        {
            itPath++;
            if (itPath == m_paths.end())
            {
                if (Periodic())
                {
                    itPath = m_paths.begin();
                }
                else
                {
                    Reverse();
                    itPath = m_paths.end() - 1;
                }
            }
        }
        return(itPath);
    }
    else
    {
        // Moving backwards.
        if (itPath == m_paths.begin())
        {
            if (Periodic())
            {
                itPath = m_paths.end() - 1;
            }
            else
            {
                Reverse();
            }
        }
        else
        {
            itPath--;
        }

        return(itPath);
    }
}


Cam::Point Cam::ContiguousPath::StartPoint() const
{
	if (m_paths.size() == 0) return(Cam::Point(0.0,0.0,0.0));
	if (m_is_forwards)
	{
	    return(m_paths.begin()->StartPoint());
	}
	else
	{
	    return((m_paths.end()-1)->StartPoint());
	}
}

Cam::Point Cam::ContiguousPath::EndPoint() const
{
	if (m_paths.size() == 0) return(Cam::Point(0.0,0.0,0.0));
	if (m_is_forwards)
	{
	    return((m_paths.end()-1)->EndPoint());
	}
	else
	{
	    return(m_paths.begin()->EndPoint());
	}
}


Standard_Real Cam::Path::StartParameter() const
{
	if (m_is_forwards)
	{
		return(Curve().FirstParameter());
	}
	else
	{
		return(Curve().LastParameter());
	}
}

Standard_Real Cam::Path::EndParameter() const
{
	if (m_is_forwards)
	{
		return(Curve().LastParameter());
	}
	else
	{
		return(Curve().FirstParameter());
	}
}


Bnd_Box ContiguousPath::BoundingBox() const
{
	if (m_pBoundingBox.get() == NULL)
	{
		m_pBoundingBox = std::auto_ptr<Bnd_Box>(new Bnd_Box());

		for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
		{
			std::list<Cam::Point> points = Cam::InterpolateCurve( itPath->Curve(), itPath->StartParameter(), itPath->EndParameter(), 0.001 );
			for (std::list<Cam::Point>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
			{
				m_pBoundingBox->Add(itPoint->Location());
			}		
		}
	}

	return(*m_pBoundingBox);
}

Bnd_Box Paths::BoundingBox() const
{
	Bnd_Box box;
	for (std::vector<ContiguousPath>::const_iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		box.Add( itPath->BoundingBox() );
	}

	return(box);
}


std::set<Point> Corners(Bnd_Box box)
{
	std::set<Cam::Point> corners;
	Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
	box.Get( aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
	std::list<Standard_Real> x, y, z;
	x.push_back(aXmin);
	x.push_back(aYmax);
	y.push_back(aYmin);
	y.push_back(aYmax);
	z.push_back(aZmin);
	z.push_back(aZmax);

	for (std::list<Standard_Real>::iterator itX = x.begin(); itX != x.end(); itX++)
	{
		for (std::list<Standard_Real>::iterator itY = y.begin(); itY != y.end(); itY++)
		{
			for (std::list<Standard_Real>::iterator itZ = z.begin(); itZ != z.end(); itZ++)
			{
				corners.insert( Cam::Point(*itX, *itY, *itZ) );
			}
		}
	}

	return(corners);
}

Standard_Real LargestBoxDimension(Bnd_Box box)
{
	Standard_Real largest_box_dimension = 0.0;
	std::set<Cam::Point> corners = Cam::Corners(box);
	for (std::set<Cam::Point>::iterator itOne = corners.begin(); itOne != corners.end(); itOne++)
	{
		for (std::set<Cam::Point>::iterator itTwo = corners.begin(); itTwo != corners.end(); itTwo++)
		{
			Standard_Real distance = itOne->Location().Distance( itTwo->Location() );
			if (distance > largest_box_dimension) largest_box_dimension = distance;
		}
	}

	return(largest_box_dimension);
}

/**
	When we're doing a 'point in polygon' check (in the Surrounds(point) method), we need to select
	a point on a polygon that is NOT the same as any of the corners of the bounding box.  This ensures
	that our generated line will to THROUGH the contiguous path rather than clip one corner of it.

	Use the midpoint of each segment until we find one that does not match a bounding box corner.
	We check the midpoint against each corner as the segment may be an arc, spline or bezier curve
	in which its midpoint may be at a corner of the bounding box.  If all segments are straight
	lines then the first segment tested should suffice.
 */
Cam::Point Cam::ContiguousPath::MidpointForSurroundsCheck() const
{
	Bnd_Box box = BoundingBox();
	std::set<Cam::Point> corners = Cam::Corners(box);

	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		Cam::Point midpoint = itPath->MidPoint();
		if (corners.find( midpoint ) == corners.end())
		{
			return(midpoint);
		}
	}

	return(StartPoint());	// Shouldn't get here.
}

/**
	This method returns TRUE if this contiguous path completely surrounds the rhs path.
	We decide this by checking that the two paths don't have any intersecting points
	and at least one of the rhs endpoints is 'inside' this path.
 */
bool Cam::ContiguousPath::Surrounds( const Cam::ContiguousPath & rhs ) const
{
	Ids_t ids_we_surround = PathsThatWeSurround();
	Ids_t ids_that_surround_us = rhs.PathsThatSurroundUs();
	if (ids_we_surround.find(rhs.ID()) != ids_we_surround.end()) return(true);	// We'v already figured this out.
	if (ids_that_surround_us.find(ID()) != ids_that_surround_us.end()) return(true);	// We'v already figured this out.

	if (BoundingBox().IsOut(rhs.BoundingBox())) return(false);

	if (Intersect(rhs, true).size() > 0) return(false);

	/*
	for (::size_t lhs_counter = 0; lhs_counter < m_paths.size(); lhs_counter++)
	{
		for (::size_t rhs_counter = 0; rhs_counter < rhs.m_paths.size(); rhs_counter++)
		{
			IntTools_BeanBeanIntersector intersector(m_paths[lhs_counter].Edge(), rhs.m_paths[rhs_counter].Edge());
			intersector.Perform();
			if (intersector.IsDone())
			{
				if (intersector.Result().Length() > 0) return(false);
			}
		} // End for
	} // End for
	*/

	// The first test has passed.  They could be sitting next to each other.  Look at
	// any point on the RHS shape and see if it's inside this shape.
	bool surrounds = Surrounds( rhs.MidpointForSurroundsCheck() );
	if (surrounds == true)
	{
		/*
		#ifdef HEEKSCNC
		static int count=0;
		count++;
		{
			Cam::ContiguousPath temp(*this);
			HeeksObj *lhsSketch = temp.Sketch();
			QString lhsTitle;
			lhsTitle << QString::fromUtf8("lhs ") << count;
			lhsSketch->OnEditString(lhsTitle);
			heeksCAD->Add(lhsSketch,NULL);
		}
		{
			Cam::ContiguousPath temp(rhs);
			HeeksObj *lhsSketch = temp.Sketch();
			QString lhsTitle;
			lhsTitle << QString::fromUtf8("rhs ") << count;
			lhsSketch->OnEditString(lhsTitle);
			heeksCAD->Add(lhsSketch,NULL);
		}
		#endif
		*/
	}
	return( surrounds );
}

/**
	This method returns TRUE if the point is inside this contiguous path.

	NOTE: This method is only meaningful if the path is periodic (i.e. a closed shape).
	If it's not periodic then FALSE is returned as a random option.
 */
bool Cam::ContiguousPath::Surrounds(const Cam::Point point) const
{
	if (! Periodic()) return(false);	// This routine doesn't make sense for non-periodic paths.

	try
	{
		// We need to project onto a face that is large enough to hold the whole wire and we need to
		// extend a line from that face to the wire.  Use the bounding boxes to get the largest of
		// all dimensions and use that as a 'largest' value to make sure our faces and lines are
		// large/long enough.

		Standard_Real largest(0.0);
		std::list<Bnd_Box> boxes;
		boxes.push_back(BoundingBox());

		for (std::list<Bnd_Box>::iterator itBox = boxes.begin(); itBox != boxes.end(); itBox++)
		{
			Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
			itBox->Get(aXmin,aYmin,aZmin,aXmax,aYmax,aZmax);

			double x = aXmax - aXmin;
			double y = aYmax - aYmin;
			double z = aZmax - aZmin;
			
			if (x > largest) largest = x;
			if (y > largest) largest = y;
			if (z > largest) largest = z;
		}

		// Project the right hand edge onto this object's plane before looking for intersections.
		BRepBuilderAPI_MakeFace face_maker(Plane(), -2.0 * largest, +2.0 * largest, -2.0 * largest, +2.0 * largest);
		if (face_maker.IsDone())
		{
			BRepProj_Projection projection( Wire(), face_maker.Face(), Plane().Axis().Direction() );
			if (projection.IsDone())
			{
				Cam::Paths temp;
				while (projection.More())
				{
					temp.Add( projection.Current() );
					projection.Next();
				} // End while

				if (temp.size() > 0)
				{
					// Bnd_Box box = temp[0].BoundingBox();

					// Construct a line from this point to a little past the edge of the bounding box.  We then
					// want to intersect this line with every path within this ContiguousPath object.  If we end up
					// with an even number of intersections then the point is outside the ContiguousPath.  Otherwise
					// it's inside it.

					BRep_Builder aBuilder;
					TopoDS_Vertex start, end;

					Cam::Point adjusted_point(point);
					adjusted_point.SetZ( StartPoint().Z() );

					aBuilder.MakeVertex (start, adjusted_point.Location(), Cam::GetTolerance());
					start.Orientation (TopAbs_REVERSED);

					// Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
					// box.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);

					Cam::Point best_end_point = MidpointForSurroundsCheck();
					best_end_point.SetZ( StartPoint().Z() );

					gp_Vec direction( adjusted_point.Location(), best_end_point.Location() );
					Standard_Real test_line_length = 10.0 * Cam::LargestBoxDimension(temp[0].BoundingBox());
					
					aBuilder.MakeVertex (end, gp_Pnt(adjusted_point.Location().XYZ() + (direction.XYZ() * test_line_length)), Cam::GetTolerance());
					end.Orientation (TopAbs_FORWARD);

					BRepBuilderAPI_MakeEdge edge(start, end);
					Cam::Paths line;
					line.Add(edge.Edge());

					/*
					#ifdef HEEKSCNC
					if (count == 3)
					{
						heeksCAD->Add( line.Sketch(), NULL );
						heeksCAD->Add( temp.Sketch(), NULL );
					}
					#endif
					*/

					// Now accumulate all the intersection points.
					std::set<Cam::Point> intersections = temp[0].Intersect(line[0], false);

					/*
					for (std::list<Cam::Point>::iterator itPoint = intersections.begin(); itPoint != intersections.end(); itPoint++)
					{
					#ifdef HEEKSCNC
						if (count == 3)
						{
							double p[3];
							itPoint->ToDoubleArray(p);
							heeksCAD->Add( heeksCAD->NewPoint(p), NULL );
						}
					#endif
					}
					*/

					if (intersections.size() == 0) return(false);
					return((intersections.size() % 2) != 0);
				}
				else
				{
					return(false);
				}
			}
		}
	} // End try
	catch (Standard_Failure & error) 
	{
		(void) error;	// Avoid the compiler warning.
		Handle_Standard_Failure e = Standard_Failure::Caught();
	} // End catch

	return(false);	
}

/**
	Return a list of distinct intersection points between this and the rhs path.
 */
std::set<Cam::Point> Cam::Path::Intersect( const Cam::Path & rhs, const bool stop_after_first_point /* = false */ ) const
{
	// Return all intersections points between this and the rhs path objects.
	std::set<Cam::Point> intersections;

	try
	{
		IntTools_BeanBeanIntersector intersector(Edge(), rhs.Edge());
		intersector.Perform();
		if (intersector.IsDone())
		{
			IntTools_SequenceOfRanges ranges;
			intersector.Result(ranges);
			if (intersector.IsDone())
			{
				int num_intersections = intersector.Result().Length();
				IntTools_SequenceOfRanges ranges;
				intersector.Result(ranges);
				for (int index = 1; index <= num_intersections; index++)
				{
					BRepAdaptor_Curve curve(this->Edge());
					gp_Pnt point = curve.Value(ranges(index).First());
					intersections.insert( point );
					if (stop_after_first_point) return(intersections);
				}
			}
		}
	}
	catch (Standard_Failure & error) {
        (void) error;	// Avoid the compiler warning.
        Handle_Standard_Failure e = Standard_Failure::Caught();
    } // End catch

	return(intersections);
}


/**
	Project the wire(s) onto a plane and intersect the resultant wires.  For each of those intersections, project the point
	back up perpendicular to that plane and find the intersection points on 'this' object (only).
 */
std::set<Cam::Point> Cam::ContiguousPath::Intersect( const Cam::ContiguousPath & rhs, const bool stop_after_first_point /* = false */ ) const
{
	std::set<Cam::Point> results;

	// We need to project onto a face that is large enough to hold the whole wire and we need to
	// extend a line from that face to the wire.  Use the bounding boxes to get the largest of
	// all dimensions and use that as a 'largest' value to make sure our faces and lines are
	// large/long enough.

	Standard_Real largest(0.0);
	std::list<Bnd_Box> boxes;
	boxes.push_back(BoundingBox());
	boxes.push_back(rhs.BoundingBox());

	for (std::list<Bnd_Box>::iterator itBox = boxes.begin(); itBox != boxes.end(); itBox++)
	{
		Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
		itBox->Get(aXmin,aYmin,aZmin,aXmax,aYmax,aZmax);

		double x = aXmax - aXmin;
		double y = aYmax - aYmin;
		double z = aZmax - aZmin;
		
		if (x > largest) largest = x;
		if (y > largest) largest = y;
		if (z > largest) largest = z;
	}

	if (Plane().SquareDistance(rhs.Plane()) > largest)
	{
		largest = Plane().SquareDistance(rhs.Plane());
	}

	if (Plane().SquareDistance(rhs.Plane()) > 0.001)
	{
		try
		{
			// Project the right hand edge onto this object's plane before looking for intersections.
			BRepBuilderAPI_MakeFace face_maker(Plane(), -2.0 * largest, +2.0 * largest, -2.0 * largest, +2.0 * largest);
			if (face_maker.IsDone())
			{
				BRepProj_Projection projection( rhs.Wire(), face_maker.Face(), Plane().Axis().Direction() );
				if (projection.IsDone())
				{
					Cam::Paths temp;
					while (projection.More())
					{
						temp.Add( projection.Current() );
						projection.Next();
					} // End while

					for (::size_t i=0; i<temp.size(); i++)
					{
						std::set<Cam::Point> points = Intersect(temp[i], stop_after_first_point);
						for (std::set<Cam::Point>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
						{
							// Project these points back perpendicular to this Plane() and find where such lines intersect
							// this object.

							Cam::Point start(*itPoint);
							Cam::Point end(*itPoint);
							start.Adjustment( Plane().Axis().Direction(), -2.0 * largest );
							end.Adjustment( Plane().Axis().Direction(), +2.0 * largest );

							Cam::Path projection_line(Cam::Edge( start.Location(), end.Location() ));
							for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
							{
								std::set<Cam::Point> pts = itPath->Intersect(projection_line, stop_after_first_point);
								std::copy( pts.begin(), pts.end(), std::inserter( results, results.end() ));
								if ((pts.size() > 0) && (stop_after_first_point)) return(results);
							}
						}
					}
				}
			}
		} // End try
		catch (Standard_Failure & error) 
		{
			(void) error;	// Avoid the compiler warning.
			Handle_Standard_Failure e = Standard_Failure::Caught();
		} // End catch
	}
	else
	{
		// They're on the same plane so just intersect all the edges.
		if (BoundingBox().IsOut(rhs.BoundingBox())) return(results);

		for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
		{
			for (std::vector<Path>::const_iterator itRhsPath = rhs.m_paths.begin(); itRhsPath != rhs.m_paths.end(); itRhsPath++)
			{
				std::set<Cam::Point> pts = itPath->Intersect( *itRhsPath );
				std::copy( pts.begin(), pts.end(), std::inserter( results, results.end() ));
				if ((pts.size() > 0) && (stop_after_first_point)) return(results);
			}
		}
	}

	return(results);

} // End Intersect() method



/**
	Return a TopoDS_Wire that represents this contiguous path.
 */
TopoDS_Wire Cam::ContiguousPath::Wire() const
{
	BRepBuilderAPI_MakeWire wire_maker;
	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		TopoDS_Edge edge = itPath->Edge();
		if (Cam::IsValid(edge))
		{
			wire_maker.Add( edge );
		}
	}

	wire_maker.Build();
	if (wire_maker.IsDone())
	{
		TopoDS_Wire wire = wire_maker.Wire();
		if (Cam::IsValid(wire))
		{
			ShapeFix_Wire fix;
			fix.Load( wire );
			fix.FixReorder();
			fix.FixConnected();
			fix.FixClosed();

			return(fix.Wire());
		}
		else
		{
			TopoDS_Wire empty;
			return(empty);
		}
	}
	else
	{
		TopoDS_Wire empty;
		return(empty);
	}
}


/**
	If the cross-product of the vectors at the path endpoints are all positive then the overall path
	is counter-clockwise.
 */
bool Cam::ContiguousPath::IsClockwise()
{
	if (Periodic() == false)
	{
		// It's an 'open' shape.  Let the clockwise indicator be based on whether it's more north/south
		// or more east/west.  If the endpoints are more south to north then we will call it clockwise.
		// If it's more north to south then we will call it counter-clockwise.  This makes more sense of the
		// values in the eSide enumeration.  eg: 'eLeftOrOutside' makes sense if we match a clockwise
		// closed shape with a north to south open shape.

		if (fabs(StartPoint().X() - EndPoint().X()) > fabs( StartPoint().Y() - EndPoint().Y() ))
		{
			// It's more east/west than north/south.  Figure out whether it's left to right or right to left.
			if (StartPoint().X() < EndPoint().X())
			{
				// It's left to right.
				return(false);	// counter-clockwise
			}
			else
			{
				// It's right to left.
				return(true);
			}
		}
		else
		{
			// It's more north/south.  Figure out whether it's bottom to top or top to bottom.
			if (StartPoint().Y() < EndPoint().Y())
			{
				// It's bottom to top
				return(false); // counter-clockwise
			}
			else
			{
				// It's top to bottom.
				return(true);	// clockwise.
			}
		}
	}

	Standard_Real value = 0.0;

	std::vector<Path>::iterator first_path = m_paths.begin();
	std::vector<Path>::iterator lhs = first_path;
	do
	{
		std::vector<Path>::iterator rhs = Next(lhs);

		// Create a vector going from the lhs element pointing towards the endpoint as well
		// as another vector going from the start of the rhs element going away from the starting point.

		/*
		// Force the Z values to all zero so that the checks are all made on the same plane.

		gp_Pnt a(lhs->PointAt( lhs->Proportion(0.99) ).Location()); a.SetZ(0.0);
		gp_Pnt b(lhs->PointAt( lhs->Proportion(1.0 ) ).Location());  b.SetZ(0.0);
		gp_Vec from( a, b );

		gp_Pnt c(rhs->PointAt( rhs->Proportion(0.0) ).Location()); c.SetZ(0.0);
		gp_Pnt d(rhs->PointAt( rhs->Proportion(0.01) ).Location());  d.SetZ(0.0);
		gp_Vec to( c, d );
		*/

		gp_Vec from( lhs->PointAt( lhs->Proportion(0.99) ).Location(), lhs->PointAt( lhs->Proportion(1.0) ).Location() );
		gp_Vec to( rhs->PointAt( rhs->Proportion(0.0) ).Location(), rhs->PointAt( rhs->Proportion(0.01) ).Location() );


		// Cross these vectors and look at the sign of the Z value to decide if these are clockwise
		// or counter-clockwise
		from.Cross(to);

		value += from.Z();
		lhs = Next(lhs);
	} while (lhs != first_path);

	return(value <= 0.0);
}

void Cam::ContiguousPath::D0(const Standard_Real DistanceAlong, gp_Pnt &P)
{
	Standard_Real distance = 0.0;

	std::vector<Path>::iterator itPath = m_paths.begin();
	while ((distance + itPath->Length()) < DistanceAlong)
	{
		distance += itPath->Length();
		itPath = Next(itPath);
	} // End while

	Standard_Real remaining = DistanceAlong - distance;
	Standard_Real proportion = remaining / itPath->Length();
	Standard_Real U = ((itPath->EndParameter() - itPath->StartParameter()) * proportion) + itPath->StartParameter();
	itPath->D0(U,P);
}

void Cam::ContiguousPath::D1(const Standard_Real DistanceAlong, gp_Pnt &P, gp_Vec &V)
{
	Standard_Real distance = 0.0;

	std::vector<Path>::iterator itPath = m_paths.begin();
	while ((distance + itPath->Length()) < DistanceAlong)
	{
		distance += itPath->Length();
		itPath = Next(itPath);
	} // End while

	Standard_Real remaining = DistanceAlong - distance;
	Standard_Real proportion = remaining / itPath->Length();
	Standard_Real U = ((itPath->EndParameter() - itPath->StartParameter()) * proportion) + itPath->StartParameter();
	itPath->D1(U,P,V);
}

std::pair<gp_Pnt, gp_Vec> Cam::Path::Nearest(const gp_Pnt reference_location, Standard_Real *pParameter /* = NULL */, const bool snap_to_nearest_curve_endpoint /* = true */ ) const
{
	std::pair<gp_Pnt, gp_Vec> nearest = std::make_pair( StartPoint().Location(), gp_Vec() );
	if (pParameter) *pParameter = this->StartParameter();
	// gp_Pnt nearest = StartPoint().Location();

    Standard_Real start_u, end_u;
	Handle(Geom_Curve) curve = BRep_Tool::Curve(m_edge, start_u, end_u);
	GeomAPI_ProjectPointOnCurve projection(reference_location, curve);

	Extrema_ExtPC extrema(reference_location, GeomAdaptor_Curve(curve));
	if (extrema.IsDone())
	{
		for (int i=1; i<=extrema.NbExt(); i++)
		{
			gp_Pnt n = extrema.Point(i).Value();
			if (nearest.first.Distance(reference_location) > n.Distance(reference_location))
			{
				gp_Pnt a;
				gp_Vec vec;
				curve->D1(extrema.Point(i).Parameter(), a, vec);
				nearest = std::make_pair( a, vec );
				if (pParameter) *pParameter = extrema.Point(i).Parameter();
			}
		}
	}

	if (snap_to_nearest_curve_endpoint)
	{
		gp_Pnt start, end;
		gp_Vec start_vec, end_vec;
		curve->D1(start_u, start, start_vec);
		curve->D1(end_u, end, end_vec);

		if (nearest.first.Distance(reference_location) > start.Distance(reference_location))
		{
			nearest = std::make_pair( start, start_vec );
			if (pParameter) *pParameter = start_u;
		}

		if (nearest.first.Distance(reference_location) > end.Distance(reference_location))
		{
			nearest = std::make_pair( end, end_vec );
			if (pParameter) *pParameter = end_u;
		}
	}

	/*
	for (int i=1; i<=projection.NbPoints(); i++)
	{
		gp_Pnt n;
		Standard_Real u;
		projection.Parameter(i,u);
		curve->D0( u, n );
		if (nearest.Distance(reference_location) > n.Distance(reference_location))
		{
			nearest = n;
		}
	}
	*/

	/*
	if (projection.NbPoints() > 0)
	{
		if (nearest.Distance(reference_location) > projection.NearestPoint().Distance(reference_location))
		{
			nearest = projection.NearestPoint();
		}
	}
	*/

	return(nearest);
}

std::pair<gp_Pnt, gp_Vec> Cam::ContiguousPath::Nearest(const gp_Pnt reference_location) const
{
	std::pair<gp_Pnt, gp_Vec> nearest;
	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		if (itPath == m_paths.begin())
		{
			nearest = itPath->Nearest(reference_location);
		}
		else
		{
			std::pair<gp_Pnt, gp_Vec> next_nearest = itPath->Nearest(reference_location);
			if (nearest.first.Distance(reference_location) > next_nearest.first.Distance(reference_location))
			{
				nearest = next_nearest;
			}
		}
	}

	return(nearest);
}


Cam::ContiguousPath & Cam::ContiguousPath::operator =(const Cam::ContiguousPath &rhs)
{
	if (this != &rhs)
	{
		m_is_forwards = rhs.m_is_forwards;
		m_paths.clear();
		std::copy( rhs.m_paths.begin(), rhs.m_paths.end(), std::inserter(m_paths, m_paths.begin()) );
		m_concentricity = rhs.m_concentricity;
		m_id = rhs.m_id;
		m_paths_that_we_surround.clear();
		std::copy( rhs.m_paths_that_we_surround.begin(), rhs.m_paths_that_we_surround.end(), std::inserter(m_paths_that_we_surround, m_paths_that_we_surround.begin()) );
		m_paths_that_surround_us.clear();
		std::copy( rhs.m_paths_that_surround_us.begin(), rhs.m_paths_that_surround_us.end(), std::inserter(m_paths_that_surround_us, m_paths_that_surround_us.begin()) );
		m_plane.reset(NULL);
		m_pBoundingBox.reset(NULL);
	}

	return(*this);
}

Cam::ContiguousPath::ContiguousPath(const Cam::ContiguousPath &rhs)
{
	*this = rhs;	// Call the assignment operator.
}


Cam::Paths::Paths( const Cam::Paths & rhs )
{
	*this = rhs;	// Call the assignment operator
}

Cam::Paths & Cam::Paths::operator = ( const Cam::Paths & rhs )
{
	if (this != &rhs)
	{
		m_contiguous_paths.clear();
		std::copy( rhs.m_contiguous_paths.begin(), rhs.m_contiguous_paths.end(), std::inserter( m_contiguous_paths, m_contiguous_paths.begin() ) );
		m_is_forwards = rhs.m_is_forwards;
	}

	return(*this);
}

void Cam::ContiguousPath::Reset()
{
	m_paths_that_surround_us.clear();
	m_paths_that_we_surround.clear();
	Concentricity(0);
}

Cam::ContiguousPath Cam::Paths::GetByID( const Cam::ContiguousPath::Id_t id ) const
{
	for (std::vector<ContiguousPath>::const_iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		if (itPath->ID() == id) return(*itPath);
	}

	throw(std::runtime_error("No contiguous paths found for ID"));
}

gp_Pln Cam::ContiguousPath::Plane() const
{
	if (m_plane.get()) return(*m_plane);
	
	TopoDS_Wire wire = Wire();
	if (Cam::IsValid(wire))
	{
		BRepBuilderAPI_FindPlane findPlane(wire, Cam::GetTolerance());
		if (!findPlane.Found())
		{
			m_plane = std::auto_ptr<gp_Pln>(new gp_Pln(gp_Ax3(gp::XOY())));	// Default to XY plane
			m_plane->Translate(gp_Vec(m_plane->Location(), StartPoint().Location())); // But at the same height as the linear element.
			return(*m_plane);
		}
		else
		{
			Handle(Geom_Plane) hPlane = findPlane.Plane();
			m_plane = std::auto_ptr<gp_Pln>(new gp_Pln(hPlane->Pln()));	// Default to XY plane

			// If this plane's 'normal' direction (typically the 'z' axis) is pointing in the negative
			// 'z' direction then reverse the plane and use that.

			if (m_plane->Axis().Direction().Z() < 0.0)
			{
				gp_Ax1 ax1 = m_plane->Axis();
				ax1.Reverse();
				m_plane->SetAxis( ax1 );
			}

			return(*m_plane);
		}
	}
	else
	{
		m_plane = std::auto_ptr<gp_Pln>(new gp_Pln(gp_Ax3(gp::XOY())));	// Default to XY plane
		m_plane->Translate(gp_Vec(m_plane->Location(), StartPoint().Location())); // But at the same height as the linear element.
		return(*m_plane);
	}
	
	
}


/*
TopoDS_Shape TopoShape::makeOffsetShape(double offset, double tol, bool intersection,
                                        bool selfInter, short offsetMode, short join,
                                        bool fill) const
{
    BRepOffsetAPI_MakeOffsetShape mkOffset(this->_Shape, offset, tol, BRepOffset_Mode(offsetMode),
        intersection ? Standard_True : Standard_False,
        selfInter ? Standard_True : Standard_False,
        GeomAbs_JoinType(join));
    const TopoDS_Shape& res = mkOffset.Shape();
    if (!fill)
        return res;

    //s=Part.makePlane(10,10)
    //s.makeOffsetShape(1.0,0.01,False,False,0,0,True)
    const BRepOffset_MakeOffset& off = mkOffset.MakeOffset();
    const BRepAlgo_Image& img = off.OffsetEdgesFromShapes();

    // build up map edge->face
    TopTools_IndexedDataMapOfShapeListOfShape edge2Face;
    TopExp::MapShapesAndAncestors(this->_Shape, TopAbs_EDGE, TopAbs_FACE, edge2Face);
    TopTools_IndexedMapOfShape mapOfShape;
    TopExp::MapShapes(this->_Shape, TopAbs_EDGE, mapOfShape);

    TopoDS_Shell shell;
    BRep_Builder builder;
    TopExp_Explorer xp;
    builder.MakeShell(shell);

    for (xp.Init(this->_Shape,TopAbs_FACE); xp.More(); xp.Next()) {
        builder.Add(shell, xp.Current());
    } 

    for (int i=1; i<= edge2Face.Extent(); ++i) {
        const TopTools_ListOfShape& los = edge2Face.FindFromIndex(i);
        if (los.Extent() == 1) {
            // set the index value as user data to use it in accept()
            const TopoDS_Shape& edge = edge2Face.FindKey(i);
            Standard_Boolean ok = img.HasImage(edge);
            if (ok) {
                const TopTools_ListOfShape& edges = img.Image(edge);
                TopTools_ListIteratorOfListOfShape it;
                it.Initialize(edges);
                BRepOffsetAPI_ThruSections aGenerator (0,0);
                aGenerator.AddWire(BRepBuilderAPI_MakeWire(TopoDS::Edge(edge)).Wire());
                aGenerator.AddWire(BRepBuilderAPI_MakeWire(TopoDS::Edge(it.Value())).Wire());
                aGenerator.Build();
                for (xp.Init(aGenerator.Shape(),TopAbs_FACE); xp.More(); xp.Next()) {
                    builder.Add(shell, xp.Current());
                }
                //TopoDS_Face face = BRepFill::Face(TopoDS::Edge(edge), TopoDS::Edge(it.Value()));
                //builder.Add(shell, face);
            }
        }
    }

    for (xp.Init(mkOffset.Shape(),TopAbs_FACE); xp.More(); xp.Next()) {
        builder.Add(shell, xp.Current());
    } 

    //BRepBuilderAPI_Sewing sew(offset);
    //sew.Load(this->_Shape);
    //sew.Add(mkOffset.Shape());
    //sew.Perform();

    //shell.Closed(Standard_True);

    return shell;
}
*/


/**
	returns 'true' if all edges represented by the shape (whatever it is) have length
	and are non-null.
 */
bool IsValid(const TopoDS_Shape shape)
{
	if (shape.IsNull()) return(false);

	switch (shape.ShapeType())
	{
	case TopAbs_EDGE:
		{
		BRepAdaptor_Curve curve(TopoDS::Edge(shape));
		gp_Pnt half_way = curve.Value(((curve.LastParameter() - curve.FirstParameter()) / 2.0) + curve.FirstParameter());
		gp_Pnt start = curve.Value(curve.FirstParameter());
		if (start.Distance(half_way) < Cam::GetTolerance()) return(false);
		}
		break;

	case TopAbs_VERTEX:
	case TopAbs_SHAPE:
		return(true);

	case TopAbs_COMPOUND:
	case TopAbs_COMPSOLID:
	case TopAbs_SOLID:
	case TopAbs_SHELL:
	case TopAbs_FACE:
	case TopAbs_WIRE:
		{
			TopoDS_Iterator It(shape, Standard_True, Standard_True);
			for (; It.More(); It.Next())
			{
				if (! IsValid(It.Value())) return(false);
			}
		}
		break;
	}

	return(true);
}



bool Paths::Offset(const double distance)
{
	// Pick the plane of the first contiguous path and assume the rest align with that.
	// The libArea offset code only works in 2D so we need to rotate to the XY plane
	// and rotate the results back again if necessary.
	if (size() == 0) return(false);
	
	gp_Pln original_plane(m_contiguous_paths[0].Plane());
	gp_Pln xy_plane = gp_Pln(gp_Ax3(gp::XOY())); // Default XY plane.

	Ax3 original_orientation(original_plane.Position());
	Ax3 xy_orientation(xy_plane.Position());

	if (original_orientation != xy_orientation)
	{
		gp_Trsf transformation;
		transformation.SetTransformation( original_plane.Position(), xy_plane.Position() );
		transformation.Invert();
		Transform(transformation);
	}

	area::CArea open;
	area::CArea closed;

	Sort();	// we MUST have the concentricity values set correctly
			// in order for the clockwise/counter-clockwise ordering
			// required within the libArea code.

	for (::size_t i=0; i<size(); i++)
	{
		if (m_contiguous_paths[i].Periodic())
		{
			closed.append( m_contiguous_paths[i].AreaCurve() );
		}
		else
		{
			open.append( m_contiguous_paths[i].AreaCurve() );
		}
	}

	m_contiguous_paths.clear();

	if (closed.num_curves() > 0)
	{
		closed.Offset(double(distance * -1.0));	// the logic of distance is reversed in the libArea offset routine.
		*this = Cam::Paths(closed);
	}

	if (open.num_curves() > 0)
	{
		for (std::list<area::CCurve>::iterator itCurve = open.m_curves.begin(); itCurve != open.m_curves.end(); itCurve++)
		{
			itCurve->Offset(double(distance));
		}

		Cam::Paths temp(open);
		for (::size_t i=0; i<temp.size(); i++)
		{
			Add( temp[i].Wire() );
		}
	}

	if (original_orientation != xy_orientation)
	{
		gp_Trsf transformation;
		transformation.SetTransformation( xy_plane.Position(), original_plane.Position() );
		transformation.Invert();
		Transform(transformation);
	}

	return(m_contiguous_paths.size() > 0);
}

bool Ax3::operator== ( const Cam::Ax3 & rhs ) const
{
	if (this != &rhs)
	{
		if (CDouble(gp_Ax3::XDirection().XYZ().X()) != CDouble(rhs.XDirection().XYZ().X())) return(false);
		if (CDouble(gp_Ax3::XDirection().XYZ().Y()) != CDouble(rhs.XDirection().XYZ().Y())) return(false);
		if (CDouble(gp_Ax3::XDirection().XYZ().Z()) != CDouble(rhs.XDirection().XYZ().Z())) return(false);

		if (CDouble(gp_Ax3::YDirection().XYZ().X()) != CDouble(rhs.YDirection().XYZ().X())) return(false);
		if (CDouble(gp_Ax3::YDirection().XYZ().Y()) != CDouble(rhs.YDirection().XYZ().Y())) return(false);
		if (CDouble(gp_Ax3::YDirection().XYZ().Z()) != CDouble(rhs.YDirection().XYZ().Z())) return(false);

		if (CDouble(gp_Ax3::Direction().XYZ().X()) != CDouble(rhs.Direction().XYZ().X())) return(false);
		if (CDouble(gp_Ax3::Direction().XYZ().Y()) != CDouble(rhs.Direction().XYZ().Y())) return(false);
		if (CDouble(gp_Ax3::Direction().XYZ().Z()) != CDouble(rhs.Direction().XYZ().Z())) return(false);

		return(true);
	}
	else
	{
		return(true);
	}
}

bool Ax3::operator!= ( const Cam::Ax3 & rhs ) const
{
	if (this != &rhs)
	{
		return(! (*this == rhs));
	}
	else
	{
		return(true);
	}
}

#ifdef HEEKSCNC
void Paths::Align( CFixture fixture )
{
	for (std::vector<ContiguousPath>::iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		itPath->Align( fixture );
	} // End for
}

void ContiguousPath::Align( CFixture fixture )
{
	for (std::vector<Path>::iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		itPath->Align( fixture );
	} // End for
	m_plane.reset(NULL);
	m_pBoundingBox.reset(NULL);
}

void Path::Align( CFixture fixture )
{
	TopoDS_Shape shape = fixture.Adjustment( Edge() );
	*this = Path(TopoDS::Edge(shape));
}
#endif // HEEKSCNC


Path Path::Section( const Standard_Real start_distance, const Standard_Real end_distance ) const
{
	Standard_Real start_u = Proportion(start_distance / Length());
	Standard_Real end_u   = Proportion(end_distance / Length());
	
	return(Path(Cam::Edge( Edge(), start_u, end_u )));
}

TopoDS_Edge Edge( const TopoDS_Edge original_edge, const Standard_Real start_u, const Standard_Real end_u )
{
	TopoDS_Edge empty;
	try
	{
		// BRepAdaptor_Curve curve(
		BRepAdaptor_Curve curve(original_edge);
		Handle_Adaptor3d_HCurve hCurve = curve.Trim(start_u, end_u, Cam::GetTolerance());
		switch (hCurve->Curve().GetType())
		{
		case GeomAbs_Line:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().Line(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_Circle:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().Circle(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_Ellipse:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().Ellipse(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_Hyperbola:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().Hyperbola(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_Parabola:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().Parabola(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_BezierCurve:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().Bezier(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_BSplineCurve:
			{
				BRepBuilderAPI_MakeEdge trimmed_edge(hCurve->Curve().BSpline(), hCurve->Curve().FirstParameter(), hCurve->Curve().LastParameter());
				return(trimmed_edge);
			}
			break;

		case GeomAbs_OtherCurve:
			return(empty);
		}

		return(empty);
	} // End try
	catch (Standard_Failure & error) {
		(void) error;	// Avoid the compiler warning.
		Handle_Standard_Failure e = Standard_Failure::Caught();
		// wxMessageBox(Ctt(e->GetMessageString()));
		return(empty);
	} // End catch
}

ContiguousPath ContiguousPath::Section( const Standard_Real start_distance, const Standard_Real end_distance )
{
	ContiguousPath new_contiguous_path;

	Standard_Real distance = 0.0;
	std::vector<Path>::iterator itPath = m_paths.begin();
	while (distance < start_distance)
	{
		if ((distance + itPath->Length()) > start_distance)
		{
			if ((distance + itPath->Length()) > end_distance)
			{
				new_contiguous_path.Add(itPath->Section(start_distance - distance, end_distance - distance));
				distance = end_distance;
			}
			else
			{
				new_contiguous_path.Add(itPath->Section(start_distance - distance, itPath->Length()));
				distance += itPath->Length();
			}
		}
		else
		{
			distance += itPath->Length();
		}
		itPath = Next(itPath);
	}

	while (distance < end_distance)
	{
		if ((distance + itPath->Length()) > end_distance)
		{
			new_contiguous_path.Add(itPath->Section(0.0, end_distance - distance));
			distance = end_distance;
		}
		else
		{
			new_contiguous_path.Add(itPath->Section(0.0, itPath->Length()));
			distance += itPath->Length();
		}
		itPath = Next(itPath);
	}

	return(new_contiguous_path);
}


double deflection( const Cam::Point start, const Cam::Point middle, const Cam::Point end )
{
	if ((start == middle) && (middle == end))
	{
		// It's a zero length curve.
		return(0.0);
	}

	if (start == end)
	{
		// It may be a periodic curve (eg: a spline or a circle)
		// The middle is different to the two ends so it must
		// be something worth looking at.  Return a large
		// value to force the InterpolaceCurve() method to
		// breakup the curve into smaller sections.

		return(9999999.9);	// Something larger than tolerance.
	}

    gp_Lin line(start.Location(), gp_Dir(end.Location().X() - start.Location().X(), end.Location().Y() - start.Location().Y(), end.Location().Z() - start.Location().Z()));
    return(line.SquareDistance(middle.Location()));
}

// Recursively calculate the start, middle and end points for the curve.  If the distance from the middle point to the
// line joining the start and end points is too large then recursively call this routine for each of the two
// halves of the curve.
std::list<Point> InterpolateCurve( BRepAdaptor_Curve curve, const Standard_Real start_u, const Standard_Real end_u, const double max_deviation )
{
	std::list<Cam::Point> points;
	
	Standard_Real middle_u = ((end_u - start_u) / 2.0) + start_u;
	gp_Pnt start, middle, end;
	curve.D0( start_u, start );
	curve.D0( middle_u, middle );
	curve.D0( end_u, end );

	if (Cam::deflection( start, middle, end ) >= max_deviation)
	{
		int num_sections = 2;
		for (int i=1; i<=num_sections; i++)
		{
			if (end_u > start_u)
			{
				Standard_Real one_u = (end_u - start_u) / double(num_sections);
				Standard_Real u1 = start_u + (double(i-1) * one_u);
				Standard_Real u2 = start_u + (double(i) * one_u);
				std::list<Cam::Point> pts = InterpolateCurve( curve, u1, u2, max_deviation );
				std::copy( pts.begin(), pts.end(), std::inserter( points, points.end() ) );
			}
			else
			{
				Standard_Real one_u = (start_u - end_u) / double(num_sections);
				Standard_Real u1 = end_u + (double(i-1) * one_u);
				Standard_Real u2 = end_u + (double(i) * one_u);
				std::list<Cam::Point> pts = InterpolateCurve( curve, u1, u2, max_deviation );
				std::copy( pts.begin(), pts.end(), std::inserter( points, points.end() ) );
			}
		}
	}
	else
	{
		points.push_back(start);
		points.push_back(end);
	}

	// For debug only...
	/*
	#ifdef HEEKSCNC
		for (std::list<Cam::Point>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
		{
			double p[3];
			itPoint->ToDoubleArray(p);
			heeksCAD->Add( heeksCAD->NewPoint(p), NULL );
		}
	#endif
	*/

	points.unique();
	return(points);
}


/**
	Where the child path objects are lines and arcs, their offsets are also lines and arcs.  Where the child
	objects are any other type (eg: BSpline, Bezier curve etc.) then the offset shapes are converted
	into lines.  This method retains the line and arc children (if asked to do so) but converts all the
	more complex children into a series of lines that are no more than 'max_deviation' from the original
	object's path.
 */
ContiguousPath ContiguousPath::InterpolateLines(const double max_deviation, const bool retain_simple_curve_types /* = false */ ) const
{
	ContiguousPath new_path;

	// Retain this object's concentricity value as the returned path is really
	// just a new version of 'this' one.
	new_path.Concentricity( Concentricity() );

	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		GeomAbs_CurveType curve_type = itPath->Curve().GetType();
		if (retain_simple_curve_types && ((curve_type == GeomAbs_Circle) || (curve_type == GeomAbs_Line)))
		{
			new_path.Add( itPath->Edge() );		
		}
		else
		{
			std::list<Cam::Point> points = Cam::InterpolateCurve( itPath->Curve(), itPath->StartParameter(), itPath->EndParameter(), max_deviation );
			std::list<Cam::Point>::iterator itPrevious;
			for (std::list<Cam::Point>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
			{
				if(itPoint != points.begin())
				{
					if (*itPoint != *itPrevious)
					{
						new_path.Add(Edge(itPrevious->Location(), itPoint->Location()));
						itPrevious = itPoint;
					}
				}
				else
				{
					itPrevious = itPoint;
				}
			} // End for
		}
	} // End for

	return(new_path);
}

Paths & Paths::operator-= ( const Paths & rhs )
{
	if (this != &rhs)
	{
		std::list<TopoDS_Face> subtrahend;
		for (::size_t i=0; i<rhs.size(); i++)
		{
			subtrahend.push_back(BRepBuilderAPI_MakeFace(rhs[i].Wire()));
		}

		std::list<TopoDS_Face> minuend;
		for (::size_t i=0; i<size(); i++)
		{
			minuend.push_back(BRepBuilderAPI_MakeFace(m_contiguous_paths[i].Wire()));
		}

		// difference = minuend - subtrahend
		bool differences_found = false;		
		for (std::list<TopoDS_Face>::iterator itSubtrahend = subtrahend.begin(); itSubtrahend != subtrahend.end(); itSubtrahend++)
		{
			std::list<TopoDS_Face> difference;
			for (std::list<TopoDS_Face>::iterator itMinuend = minuend.begin(); itMinuend != minuend.end(); itMinuend++)
			{
				// We have two faces.  Offer the option to do various boolean operations between them.
				if (FacesIntersect( *itMinuend, *itSubtrahend ))
				{
					std::list<TopoDS_Face> this_difference = SubtractFaces( *itMinuend, *itSubtrahend );
					std::copy( this_difference.begin(), this_difference.end(), std::inserter( difference, difference.end() ));
					differences_found = true;
				}
				else
				{
					difference.push_back(*itMinuend);
				}
			} // End for

			minuend.clear();
			std::copy( difference.begin(), difference.end(), std::inserter( minuend, minuend.end() ));
		} // End for

		m_contiguous_paths.clear();
		if (differences_found)
		{
			for (std::list<TopoDS_Face>::iterator itFace = minuend.begin(); itFace != minuend.end(); itFace++)
			{
				Add(*itFace);
			}
		}
	} // End if - then

	return(*this);
}


static void AddVertex( const int type, const gp_Pnt point, const double tolerance, std::list<area::CVertex> *pList )
{
	if (pList)
	{
		if (pList->size() == 0)
		{
			area::CVertex vertex;
			vertex.m_type = type;
			vertex.m_user_data = 0;
			vertex.m_p.x = point.X();
			vertex.m_p.y = point.Y();
			vertex.m_p.tolerance = tolerance;
			pList->push_back(vertex);
		}
		else
		{
			if (gp_Pnt(pList->rbegin()->m_p.x, pList->rbegin()->m_p.y, 0.0).Distance(point) > tolerance)
			{
				area::CVertex vertex;
				vertex.m_type = type;
				vertex.m_user_data = 0;
				vertex.m_p.x = point.X();
				vertex.m_p.y = point.Y();
				vertex.m_p.tolerance = tolerance;
				pList->push_back(vertex);
			}
		}
	}
}

static void AddVertex( const int type, const gp_Pnt point, const gp_Pnt centre, const double tolerance, std::list<area::CVertex> *pList )
{
	if (pList)
	{
		if (pList->size() == 0)
		{
			area::CVertex vertex;
			vertex.m_type = type;
			vertex.m_user_data = 0;
			vertex.m_p.x = point.X();
			vertex.m_p.y = point.Y();
			vertex.m_c.x = centre.X();
			vertex.m_c.y = centre.Y();
			vertex.m_p.tolerance = tolerance;
			pList->push_back(vertex);
		}
		else
		{
			if (gp_Pnt(pList->rbegin()->m_p.x, pList->rbegin()->m_p.y, 0.0).Distance(point) > tolerance)
			{
				area::CVertex vertex;
				vertex.m_type = type;
				vertex.m_user_data = 0;
				vertex.m_p.x = point.X();
				vertex.m_p.y = point.Y();
				vertex.m_c.x = centre.X();
				vertex.m_c.y = centre.Y();
				vertex.m_p.tolerance = tolerance;
				pList->push_back(vertex);
			}
		}
	}
}


std::list<area::CVertex> Path::Vertices() const
{
	std::list<area::CVertex> vertices;

	BRepAdaptor_Curve oc_curve(Edge());
	GeomAbs_CurveType curve_type = oc_curve.GetType();
	bool sense = (Edge().Orientation() == TopAbs_FORWARD);

	// if(! m_is_forwards) sense = !sense;

	switch(curve_type)
	{
		case GeomAbs_Line:
			// make a line
		{
			double uStart = oc_curve.FirstParameter();
			double uEnd = oc_curve.LastParameter();
			gp_Pnt PS;
			gp_Vec VS;
			oc_curve.D1(uStart, PS, VS);
			gp_Pnt PE;
			gp_Vec VE;
			oc_curve.D1(uEnd, PE, VE);
			if (sense)
			{
				AddVertex( 0, PS, oc_curve.Tolerance(), &vertices );
				AddVertex( 0, PE, oc_curve.Tolerance(), &vertices );				
			}
			else
			{
				AddVertex( 0, PE, oc_curve.Tolerance(), &vertices );
				AddVertex( 0, PS, oc_curve.Tolerance(), &vertices );
			}
		}
		break;

		case GeomAbs_Circle:
			// make an arc
		{
			double uStart = oc_curve.FirstParameter();
			double uEnd = oc_curve.LastParameter();
			gp_Pnt PS;
			gp_Vec VS;
			oc_curve.D1(uStart, PS, VS);
			gp_Pnt PE;
			gp_Vec VE;
			oc_curve.D1(uEnd, PE, VE);
			gp_Circ circle = oc_curve.Circle();
			gp_Ax1 axis = circle.Axis();
			if(!sense)
			{
				axis.SetDirection(-axis.Direction());
				circle.SetAxis(axis);
			}

			if(oc_curve.IsPeriodic())
			{
				for (int i=0; i<4; i++)
				{
					double angle = double(i) * M_PI / 2.0;
					gp_Pnt point(PS);
					gp_Trsf rotation;
					rotation.SetRotation( circle.Axis(), angle );
					point.Transform(rotation);
					if (vertices.size() == 0)
					{
						AddVertex( 0, point, oc_curve.Tolerance(), &vertices );	
					}
					else
					{
						AddVertex( -1, point, circle.Location(), oc_curve.Tolerance(), &vertices );	
					}
				}
			}
			else
			{
				if (sense)
				{
					AddVertex( 0, PS, circle.Location(), oc_curve.Tolerance(), &vertices );
					AddVertex( ((axis.Direction().XYZ().Z() > 0.0)?1:-1), PE, circle.Location(), oc_curve.Tolerance(), &vertices );
				}
				else
				{
					AddVertex( 0, PE, circle.Location(), oc_curve.Tolerance(), &vertices );
					AddVertex( ((axis.Direction().XYZ().Z() > 0.0)?1:-1), PS, circle.Location(), oc_curve.Tolerance(), &vertices );
				}
			}
		}
		break;

		default:
		{
			// make lots of small lines
			std::list<Cam::Point> points = InterpolateCurve( Curve(), StartParameter(), EndParameter(), 0.0254 );
			std::list<Cam::Point>::iterator itPrevious;
			if (! sense) points.reverse();
			for (std::list<Cam::Point>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
			{
				if (itPoint != points.begin())
				{
					if (*itPoint != *itPrevious)
					{
						AddVertex(0, itPoint->Location(), oc_curve.Tolerance(), &vertices);
					}

					itPrevious = itPoint;
				}
				else
				{
					AddVertex(0, itPoint->Location(), oc_curve.Tolerance(), &vertices);
					itPrevious = itPoint;
				}
			}
		}
		break;
	}

	return(vertices);
}

area::CCurve ContiguousPath::AreaCurve()
{
	area::CCurve curve;
	
	std::set< std::vector<Path>::iterator > processed_paths;
	std::vector<Path>::iterator itPath = m_paths.begin();
	do
	{
		if (processed_paths.find(itPath) == processed_paths.end())
		{
			std::list<area::CVertex> vertices = itPath->Vertices();
			for (std::list<area::CVertex>::iterator itVertex = vertices.begin(); itVertex != vertices.end(); itVertex++)
			{
				if ((curve.m_vertices.size() == 0) ||
					(gp_Pnt(curve.m_vertices.rbegin()->m_p.x, curve.m_vertices.rbegin()->m_p.y, 0.0).Distance( gp_Pnt( itVertex->m_p.x, itVertex->m_p.y, 0.0) ) > itVertex->m_p.tolerance))
				{
					curve.append(*itVertex);
				}
			} // End for
			processed_paths.insert( itPath );
		} // End if -then

		itPath = Next(itPath);
	} while (processed_paths.size() < m_paths.size());

	if ((Concentricity() % 2) == 0)
	{
		// it's an 'outer' path.
		if (curve.IsClockwise())
		{
			curve.Reverse(); // outer paths must be counter clockwise in the libArea representation.
		}
	}
	else
	{
		// it's an 'inner' path.
		if (! curve.IsClockwise())
		{
			curve.Reverse(); // inner paths must be clockwise in the libArea representation.
		}
	}

	return(curve);
} // End AreaCurve() method


area::CArea Paths::Area()
{
	Sort();	// we MUST have the concentricity values set correctly
			// in order for the clockwise/counter-clockwise ordering
			// required within the libArea code.

	area::CArea area;

	for (::size_t i=0; i<size(); i++)
	{
		area.append( m_contiguous_paths[i].AreaCurve() );		
	} // End for

	return(area);
}

void Cam::Paths::Transform( const gp_Trsf transformation )
{
	for (::size_t i=0; i<size(); i++)
	{
		m_contiguous_paths[i].Transform( transformation );
	}
}


void Cam::ContiguousPath::Transform( const gp_Trsf transformation )
{
	for (std::vector<Path>::iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		itPath->Transform( transformation );
	}
	m_plane.reset(NULL);
	m_pBoundingBox.reset(NULL);
}

void Cam::Path::Transform( const gp_Trsf transformation )
{
	if (transformation.Form() != gp_Identity)
	{
		BRepBuilderAPI_Transform transform(transformation);
		transform.Perform(m_edge, false);
		m_edge = TopoDS::Edge(transform.Shape());
		m_pCurve.reset(NULL);
		m_length = -1;
	} // End if - then
}

/**
	Add the portions of the ContiguousPath children from 'this' object into either the inside or outside
	Paths objects based on whether they are within the 'areas' paths passed in.
 */
void Cam::Paths::Split( const Cam::Paths &areas, Cam::Paths *pInside, Cam::Paths *pOutside ) const
{
	for (std::vector<ContiguousPath>::const_iterator itContiguousPath = m_contiguous_paths.begin(); itContiguousPath != m_contiguous_paths.end(); itContiguousPath++)
	{
		itContiguousPath->Split( areas, pInside, pOutside );
	} // End for
}

void Cam::ContiguousPath::Split( const Cam::Paths &area, Cam::Paths *pInside, Cam::Paths *pOutside ) const
{
	Cam::ContiguousPath copy(*this);
	bool handled = false;
	for (::size_t i=0; i<area.size(); i++)
	{
		ContiguousPath cpath(area[i]);

		if (! cpath.Periodic()) continue;

		if ((! handled) && (cpath.Surrounds( *this )))
		{
			std::vector<Cam::Path> paths = copy.Paths();
			for (std::vector<Cam::Path>::iterator itPath = paths.begin(); itPath != paths.end(); itPath++)
			{
				pInside->Add( *itPath );
			}
			handled = true;
		}
	}

	if (! handled)
	{
		// It's possible that this contiguous path cross one or more of the areas passed in.  Look for that now.
		std::vector<Cam::Path> paths = copy.Paths();
		for (std::vector<Cam::Path>::iterator itPath = paths.begin(); itPath != paths.end(); itPath++)
		{
			itPath->Split( area, pInside, pOutside );
		}			
	}
}


void Cam::Path::Split( const Cam::Paths &area, Cam::Paths *pInside, Cam::Paths *pOutside ) const
{
	for (::size_t i=0; i<area.size(); i++)
	{
		ContiguousPath cpath = area[i];

		if (! cpath.Periodic()) continue;

		Cam::ContiguousPath test_path;
		test_path.Add(*this);
		if (cpath.Surrounds( test_path ))
		{
			pInside->Add( *this );
		}
		else
		{
			std::set<Point> points = cpath.Intersect(test_path, false);
			if (points.size() > 0)
			{
				// Add the start and end points so we're sure to have a complete
				// set for use when breaking it up.
				points.insert( StartPoint() );
				points.insert( EndPoint() );

				// We now need to get a list of start/end U pairs between these
				// points.
				std::map< Standard_Real, Cam::Point > sections;
				for (std::set<Point>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
				{
					Standard_Real u = -1;
					Nearest( itPoint->Location(), &u, false );
					sections.insert( std::make_pair( u, *itPoint ) );
				}

				std::vector< Standard_Real > u_values;
				for (std::map< Standard_Real, Cam::Point >::iterator itSection = sections.begin(); itSection != sections.end(); itSection++)
				{
					u_values.push_back( itSection->first );
				}
				
				// Now we have an ordered list of U/Point pairs that define the sections of the
				// path.  We now need to assign those sections to the 'inside' and 'outside' paths
				// as appropriate.

				for (std::vector<Standard_Real>::size_type i=0; i<u_values.size()-1; i++)
				{
					if (u_values[i] == u_values[i+1]) continue;
					TopoDS_Edge edge = Cam::Edge( Edge(), u_values[i], u_values[i+1] );
					if (Cam::IsValid(edge))
					{
						Path section = Path(edge);
						if (cpath.Surrounds( section.MidPoint() ))
						{
							pInside->Add( section );
						}
						else
						{
							pOutside->Add( section );
						}
					}
				}
			}
			else
			{
				// It's not inside and it doesn't intersect.
				pOutside->Add( *this );
			}
		}
	}
}


void Cam::ContiguousPath::PathsThatSurroundUs(const Cam::ContiguousPath::Id_t id) const
{
	m_paths_that_surround_us.insert(id);
}

void Cam::ContiguousPath::PathsThatWeSurround( const Cam::ContiguousPath::Id_t id ) const
{
	m_paths_that_we_surround.insert(id);
}

std::set<int> Cam::Paths::GetConcentricities() const
{
	std::set<int> concentricities;

	for (std::vector<ContiguousPath>::const_iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		concentricities.insert( itPath->Concentricity() );
	}

	return(concentricities);
}


QString Cam::Paths::KiCadBoardOutline(const int layer /* = 28 */, const int trace_width /* = 150 */) const
{
	QString description;

	for (std::vector<ContiguousPath>::const_iterator itPath = m_contiguous_paths.begin(); itPath != m_contiguous_paths.end(); itPath++)
	{
		// The board outline MUST be defined as a series of straight lines.
		Cam::ContiguousPath lines(itPath->InterpolateLines(0.01 * 25.4, false));
		description.append( lines.KiCadBoardOutline(layer,trace_width) );
	}

	return(description);
}

QString Cam::ContiguousPath::KiCadBoardOutline(const int layer, const int trace_width) const
{
	QString description;

	for (std::vector<Path>::const_iterator itPath = m_paths.begin(); itPath != m_paths.end(); itPath++)
	{
		description += itPath->KiCadBoardOutline(layer, trace_width);
	}

	return(description);
}

/**
	By the time we get here, we KNOW that we will ONLY contain straight lines.
 */
QString Cam::Path::KiCadBoardOutline(const int layer, const int trace_width) const
{
	switch (Curve().GetType())
	{
	case GeomAbs_Line:
		{
			QString description;
			description = QString::fromUtf8("$DRAWSEGMENT\n")
						+ QString::fromUtf8("Po 0 ")
						+ QString().arg(int((StartPoint().Location().X() / 25.4) * 10000.0)) + QString::fromUtf8(" ")
						+ QString().arg(int(((-1.0 * StartPoint().Location().Y()) / 25.4) * 10000.0)) + QString::fromUtf8(" ")
						+ QString().arg(int((EndPoint().Location().X() / 25.4) * 10000.0)) + QString::fromUtf8(" ")
						+ QString().arg(int(((-1.0 * EndPoint().Location().Y() / 25.4)) * 10000.0)) + QString::fromUtf8(" ")
						+ QString().arg(trace_width) + QString::fromUtf8("\n")
						+ QString::fromUtf8("De ") + QString().arg(layer) + QString::fromUtf8(" 0 900 0 0\n")
						+ QString::fromUtf8("$EndDRAWSEGMENT\n");
			return(description);
		}
		break;

default:
		return(QString::fromUtf8(""));	// Should NEVER get here.
	}
}


/**
	Return a list of faces for all closed shapes in this object.
	If the 'subtract_nested_faces' flag is true then those closed
	shapes that are 'inside' other shapes will be subtracted from
	those 'outer' shapes so that the faces effectively have holes
	in them.
 */
Cam::Faces_t Cam::Paths::Faces(const bool subtract_nested_faces /* = true */ ) const
{
	Paths closed;
	for (::size_t i=0; i<size(); i++)
	{
		Cam::ContiguousPath cpath(this->m_contiguous_paths[i]);
		if (cpath.Periodic())
		{
			closed.Add( cpath.Wire() );
		}
	}

	// Now accumulate all the closed shapes into a distinct list of faces (with internal shapes
	// cutting holes in the faces)
	closed.Sort();

	std::set<int> concentricities;
	for (::size_t i=0; i<closed.size(); i++)
	{
		Cam::ContiguousPath cpath(closed[i]);
		concentricities.insert( cpath.Concentricity() );
	} // End for

	std::list<TopoDS_Face> faces;
	for (std::set<int>::iterator itConcentricity = concentricities.begin(); itConcentricity != concentricities.end(); itConcentricity++)
	{
		for (::size_t i=0; i<closed.size(); i++)
		{
			Cam::ContiguousPath cpath(closed[i]);
			if (cpath.Concentricity() != *itConcentricity) continue;

			BRepBuilderAPI_MakeFace face_maker(cpath.Wire());
			face_maker.Build();
			if (face_maker.IsDone())
			{
				TopoDS_Face face(TopoDS::Face(face_maker.Shape()));

				if (((cpath.Concentricity() % 2) == 0) || (subtract_nested_faces == false))
				{
					// It's an outer one.  Add it to the faces we already have.
					if ((faces.size() == 0) || (subtract_nested_faces == false))
					{
						faces.push_back(face);
					}
					else
					{
						for (std::list<TopoDS_Face>::iterator itFace = faces.begin(); itFace != faces.end(); itFace++)
						{
							try
							{
								TopoDS_Shape result = BRepAlgoAPI_Fuse(*itFace, face );
								if (result.IsNull() == false)
								{
									*itFace = TopoDS::Face(result);
								}
							}
							catch (Standard_Failure) 
							{
								// Ignore the failure here.
								Handle_Standard_Failure e = Standard_Failure::Caught();
							}
						}
					}
				}
				else
				{
					// It's an inner one.  Subtract it from the faces we already have.
					for (std::list<TopoDS_Face>::iterator itFace = faces.begin(); itFace != faces.end(); itFace++)
					{
						BRepAlgoAPI_Cut cut( *itFace, face );
						cut.Build();
						if (cut.IsDone())
						{
							try
							{
								TopoDS_Shape shape = cut.Shape();

								for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
								{
									TopoDS_Face aFace = TopoDS::Face(expFace.Current());
									*itFace = aFace;
								}
							}
							catch (Standard_Failure) 
							{
								// Ignore the failure here.
								Handle_Standard_Failure e = Standard_Failure::Caught();
							}
						} // End if - then
					}
				}
			} // End if - then
		} // End for
	} // End for

	return(faces);

} // End Faces() method


/**
	This method returns a vector of Points that represent point locations from the
	Paths held internally.  These locations come from;
		- the intersections of one ContiguousPath with another
		- the centre-point of a ContiguousPath that does NOT intersect any others and
		  contains a single circle/arc path child.  In this case, the centre of the
		  arc/circle is used as the location.

	This method is used to find individual locations for Drilling, Positioning,
	Tapping or Counterbore operations. (and any other machining operations that
	act around a single location)
 */
Paths::Locations_t Paths::PointLocationData(const Point reference_location_for_sorting /* = Point(0.0, 0.0, 0.0) */) const
{
	// Check to see if we've already calculated this information.

	if (m_pPointLocationData.get() != NULL)
	{
		return(*m_pPointLocationData);
	}

	std::set< Cam::Point > distinct_locations;
	std::set< ::size_t > intersecting_paths;

	for (::size_t lhs=0; lhs<size(); lhs++)
	{
		for (::size_t rhs=lhs; rhs<size(); rhs++)
		{
			if (lhs == rhs) continue;
			if (intersecting_paths.find(lhs) != intersecting_paths.end()) continue;
			if (intersecting_paths.find(rhs) != intersecting_paths.end()) continue;

			// See if these two contiguous path objects intersect each other.
			std::set<Cam::Point> intersections = m_contiguous_paths[lhs].Intersect(m_contiguous_paths[rhs]);
			if (intersections.size() > 0)
			{
				intersecting_paths.insert(lhs);
				intersecting_paths.insert(rhs);
				std::copy( intersections.begin(), intersections.end(), std::inserter(distinct_locations, distinct_locations.end()));
			}
		} // End for
	} // End for

	// Now go through all the contiugous paths that do not intersect any of the other paths.
	// We want to know if these hold just a single circle or arc path.  If so, use the
	// centre of the circle/arc as another location.
	for (::size_t i=0; i<size(); i++)
	{
		if (intersecting_paths.find(i) != intersecting_paths.end()) continue;

		Cam::ContiguousPath cpath(m_contiguous_paths[i]);	// Get our own (mutable) copy.

		// This contiguous path has NOT been involved in an intersection.
		std::vector<Cam::Path> individual_paths = cpath.Paths();
		bool non_arc_found = false;
		std::set<Cam::Point> points_for_this_path;
		for (std::vector<Cam::Path>::iterator itPath = individual_paths.begin(); itPath != individual_paths.end(); itPath++)
		{
			if (itPath->Curve().GetType() == GeomAbs_Circle)
			{
				gp_Circ circle = itPath->Curve().Circle();
				points_for_this_path.insert( circle.Position() );
			}
			else
			{
				non_arc_found = true;
			}
		}

		// We only want to include it if we end up with a single point and all
		// path elements are arcs/circles.
		if ((! non_arc_found) && (points_for_this_path.size() == 1))
		{
			distinct_locations.insert( *(points_for_this_path.begin()) );
		}
	}

	// Add any vertices contained in this object.
	for (std::vector<TopoDS_Vertex>::const_iterator itVertex = m_vertices.begin(); itVertex != m_vertices.end(); itVertex++)
	{
		distinct_locations.insert( Cam::Point(BRep_Tool::Pnt(*itVertex)) );
	} // End for

	m_pPointLocationData = std::auto_ptr<Paths::Locations_t>(new Paths::Locations_t());
	std::copy( distinct_locations.begin(), distinct_locations.end(), std::inserter( *m_pPointLocationData, m_pPointLocationData->begin() ));

	// Now sort these locations so there is less rapid movements between them.
	for (Paths::Locations_t::iterator l_itPoint = m_pPointLocationData->begin(); l_itPoint != m_pPointLocationData->end(); l_itPoint++)
	{
		if (l_itPoint == m_pPointLocationData->begin())
		{
			// It's the first point.
			sort_points_by_distance compare( reference_location_for_sorting );
			std::sort( m_pPointLocationData->begin(), m_pPointLocationData->end(), compare );
		} // End if - then
		else
		{
			// We've already begun.  Just sort based on the previous point's location.
			Paths::Locations_t::iterator l_itNextPoint = l_itPoint;
			l_itNextPoint++;

			if (l_itNextPoint != m_pPointLocationData->end())
			{
				sort_points_by_distance compare( *l_itPoint );
				std::sort( l_itNextPoint, m_pPointLocationData->end(), compare );
			} // End if - then
		} // End if - else
	} // End for
	
	return(*m_pPointLocationData);
}

} // End namespace Cam
