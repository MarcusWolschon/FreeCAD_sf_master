
#ifndef CDOUBLE_CLASS_DEFINITION
#define CDOUBLE_CLASS_DEFINITION

class CDouble
	{
	public:
		CDouble(const double value)
		{
			m_value = value;
		}

		~CDouble() { }

		CDouble( const CDouble & rhs )
		{
			*this = rhs;
		}

		CDouble & operator= ( const CDouble & rhs )
		{
			if (this != &rhs)
			{
				m_value = rhs.m_value;
			}

			return(*this);
		}

		CDouble & operator*= ( const CDouble & rhs )
		{
			m_value *= rhs.m_value;
			return(*this);
		}

		CDouble & operator/= ( const CDouble & rhs )
		{
			m_value /= rhs.m_value;
			return(*this);
		}

		CDouble & operator+= ( const CDouble & rhs )
		{
			m_value += rhs.m_value;
			return(*this);
		}

		CDouble & operator-= ( const CDouble & rhs )
		{
			m_value -= rhs.m_value;
			return(*this);
		}

		bool operator==( const CDouble & rhs ) const
		{
			if (fabs(m_value - rhs.m_value) < (2.0 * Tolerance())) return(true);
			return(false);
		}

		bool operator!=( const CDouble & rhs ) const
		{
			return(! (*this == rhs));
		}

		bool operator< (const CDouble & rhs ) const
		{
			if (*this == rhs) return(false);
			return(m_value < rhs.m_value);
		}

		bool operator<= (const CDouble & rhs ) const
		{
			if (*this == rhs) return(true);
			return(m_value < rhs.m_value);
		}

		bool operator> (const CDouble & rhs ) const
		{
			if (*this == rhs) return(false);
			return(m_value > rhs.m_value);
		}

		bool operator>= (const CDouble & rhs ) const
		{
			if (*this == rhs) return(true);
			return(m_value > rhs.m_value);
		}

		double Value() const { return(m_value); }

		double Tolerance() const
		{
			return(0.000001);	// TODO Setup tolerances properly.
			/*
			#ifdef HEEKSCNC
				return(1.0 / pow(10, double(Python::RequiredDecimalPlaces())));
			#else
				extern CHeeksCADInterface heekscad_interface;
				return(heekscad_interface.GetTolerance());
			#endif
			*/
		}

	private:
		double	m_value;
	};
#endif // CDOUBLE_CLASS_DEFINITION
