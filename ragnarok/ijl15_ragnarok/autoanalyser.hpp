#ifndef AUTOANALYSER_HPP
#define AUTOANALYSER_HPP

#include "stdafx.h"


namespace autoanalyser
{
	void addr_aid();
	void addr_gamebase();
	void addr_actorbase();
	void addr_xml_address();
	void addr_vdtvalue();
	void addr_netbase();
	void addr_recvsend();
	void addr_charname();

	void addr_cmdjmptable(const BYTE __x);
};


#endif	// #ifndef AUTOANALYSER_HPP
