#pragma once
typedef enum tagEMSConstellationType
{	
	UNKNOWN = -1,
	GPS		= 0,
	GAL	= 1,
	GLONASS			= 2,
	BEIDOU			= 3
} EMSCONSTELLATIONTYPE;
class CConstellationType
{

  public:
    EMSCONSTELLATIONTYPE get_value()
    {
        return m_value;
    }
    void set_value(EMSCONSTELLATIONTYPE v)
    {
        m_value = v;
    }
    static CConstellationType *instance()
    {
        if (!s_instance)
          s_instance = new CConstellationType;
        return s_instance;
    }

	private:
	EMSCONSTELLATIONTYPE m_value;
    static CConstellationType *s_instance;
    CConstellationType()
    {
        m_value = GPS;
    }

};

