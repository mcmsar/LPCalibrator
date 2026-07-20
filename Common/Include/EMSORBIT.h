

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Mon Jan 18 22:14:07 2038
 */
/* Compiler settings for C:\Users\joshua.rehm\common\include\EMSORBIT.IDL:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0628 
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __EMSORBIT_h__
#define __EMSORBIT_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IEMSOrbit_FWD_DEFINED__
#define __IEMSOrbit_FWD_DEFINED__
typedef interface IEMSOrbit IEMSOrbit;

#endif 	/* __IEMSOrbit_FWD_DEFINED__ */


#ifndef __IEMSOrbit2_FWD_DEFINED__
#define __IEMSOrbit2_FWD_DEFINED__
typedef interface IEMSOrbit2 IEMSOrbit2;

#endif 	/* __IEMSOrbit2_FWD_DEFINED__ */


#ifndef __IEMSOrbit3_FWD_DEFINED__
#define __IEMSOrbit3_FWD_DEFINED__
typedef interface IEMSOrbit3 IEMSOrbit3;

#endif 	/* __IEMSOrbit3_FWD_DEFINED__ */


#ifndef __IEMSOrbit4_FWD_DEFINED__
#define __IEMSOrbit4_FWD_DEFINED__
typedef interface IEMSOrbit4 IEMSOrbit4;

#endif 	/* __IEMSOrbit4_FWD_DEFINED__ */


#ifndef __IEMSOrbit5_FWD_DEFINED__
#define __IEMSOrbit5_FWD_DEFINED__
typedef interface IEMSOrbit5 IEMSOrbit5;

#endif 	/* __IEMSOrbit5_FWD_DEFINED__ */


#ifndef __IEMSOrbitInit_FWD_DEFINED__
#define __IEMSOrbitInit_FWD_DEFINED__
typedef interface IEMSOrbitInit IEMSOrbitInit;

#endif 	/* __IEMSOrbitInit_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "emstypex.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_EMSORBIT_0000_0000 */
/* [local] */ 

 
#include "emserror.h"
 
// {E8F04C91-4552-11d5-8B95-009027912866} 
DEFINE_GUID( CLSID_EMSOrbit, 0xe8f04c91, 0x4552, 0x11d5, 0x8b, 0x95, 0x0, 0x90, 0x27, 0x91, 0x28, 0x66); 
 
// {E8F04C92-4552-11d5-8B95-009027912866} 
DEFINE_GUID( IID_IEMSOrbit, 0xe8f04c92, 0x4552, 0x11d5, 0x8b, 0x95, 0x0, 0x90, 0x27, 0x91, 0x28, 0x66); 
 
// {90EFA3C3-CFD8-4a39-9321-AC2C7C18D5AA}
DEFINE_GUID( IID_IEMSOrbit2, 0x90efa3c3, 0xcfd8, 0x4a39, 0x93, 0x21, 0xac, 0x2c, 0x7c, 0x18, 0xd5, 0xaa);
 
// {31D9B372-71A0-4298-9122-F7DFFB067439}
DEFINE_GUID( IID_IEMSOrbit3, 0x31d9b372, 0x71a0, 0x4298, 0x91, 0x22, 0xf7, 0xdf, 0xfb, 0x6, 0x74, 0x39);
 
// {738265B6-9E40-44ff-B8A7-E5F681AFBA18}
DEFINE_GUID( IID_IEMSOrbit4, 0x738265b6, 0x9e40, 0x44ff, 0xb8, 0xa7, 0xe5, 0xf6, 0x81, 0xaf, 0xba, 0x18);
 
// {DEDFE2BA-F2FB-4bee-9744-207931959F40}
DEFINE_GUID( IID_IEMSOrbit5, 0xdedfe2ba, 0xf2fb, 0x4bee, 0x97, 0x44, 0x20, 0x79, 0x31, 0x95, 0x9f, 0x40);
 
// {CE4EC876-A5CF-4f82-9B7A-5362D6D363F1}
DEFINE_GUID(IID_IEMSOrbitInit, 0xce4ec876, 0xa5cf, 0x4f82, 0x9b, 0x7a, 0x53, 0x62, 0xd6, 0xd3, 0x63, 0xf1);
// Flags for SetSatelliteTLE3 
#define EMS_ORBIT_OVERRIDE_MORE_RECENT	(0x0001)
#define EMS_ORBIT_OVERRIDE_ALL			(0x0002)
#define EMS_ORBIT_POST_MANOEUVRE			(0x0004)
 


extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0000_v0_0_s_ifspec;

#ifndef __IEMSOrbit_INTERFACE_DEFINED__
#define __IEMSOrbit_INTERFACE_DEFINED__

/* interface IEMSOrbit */
/* [unique][helpstring][uuid][object] */ 

typedef struct _tagEMSTLEDATA
    {
    EMSTIME timeEpoch;
    double fXndt2o;
    double fXndd6o;
    double fBStar;
    int nOrbitNumber;
    double fCoVarMatrix[ 21 ];
    /* [switch_is][switch_type] */ union 
        {
        /* [default] */ struct 
            {
            double fInclination;
            double fRightAscNode;
            double fEccentricity;
            double fArgPerigee;
            double fMeanAnomaly;
            double fMeanMotion;
            } 	var;
        /* [case()] */ double vect[ 6 ];
        } 	elem;
    } 	EMSTLEDATA;

typedef struct _tagEMSTLEDATA *LPEMSTLEDATA;


EXTERN_C const IID IID_IEMSOrbit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E8F04C92-4552-11d5-8B95-009027912866")
    IEMSOrbit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EarthFixedOrbit( 
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InertialOrbit( 
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EarthFixedOrbitTLE( 
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InertialOrbitTLE( 
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSatelliteTLE( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [in] */ const EMSTLEDATA *pTLEData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSatelliteTLE( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpTLEData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PropogateTLE( 
            /* [in] */ EMSTLEDATA *lpTLEData,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpPropogatedTLE) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOrbitNumber( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ ULONG *lpulOrbitNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reload( 
            /* [in] */ DWORD dwFlags) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEMSOrbitVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEMSOrbit * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEMSOrbit * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEMSOrbit * This);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbit)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbit )( 
            IEMSOrbit * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbit)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbit )( 
            IEMSOrbit * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLE )( 
            IEMSOrbit * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLE )( 
            IEMSOrbit * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, SetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE )( 
            IEMSOrbit * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [in] */ const EMSTLEDATA *pTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE )( 
            IEMSOrbit * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, PropogateTLE)
        HRESULT ( STDMETHODCALLTYPE *PropogateTLE )( 
            IEMSOrbit * This,
            /* [in] */ EMSTLEDATA *lpTLEData,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpPropogatedTLE);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetOrbitNumber)
        HRESULT ( STDMETHODCALLTYPE *GetOrbitNumber )( 
            IEMSOrbit * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ ULONG *lpulOrbitNumber);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, Reload)
        HRESULT ( STDMETHODCALLTYPE *Reload )( 
            IEMSOrbit * This,
            /* [in] */ DWORD dwFlags);
        
        END_INTERFACE
    } IEMSOrbitVtbl;

    interface IEMSOrbit
    {
        CONST_VTBL struct IEMSOrbitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEMSOrbit_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEMSOrbit_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEMSOrbit_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEMSOrbit_EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit_InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit_EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit_InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit_SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData) ) 

#define IEMSOrbit_GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit_PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE)	\
    ( (This)->lpVtbl -> PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE) ) 

#define IEMSOrbit_GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber)	\
    ( (This)->lpVtbl -> GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber) ) 

#define IEMSOrbit_Reload(This,dwFlags)	\
    ( (This)->lpVtbl -> Reload(This,dwFlags) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEMSOrbit_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EMSORBIT_0000_0001 */
/* [local] */ 

typedef /* [unique] */ IEMSOrbit *LPEMSORBIT;



extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0001_v0_0_s_ifspec;

#ifndef __IEMSOrbit2_INTERFACE_DEFINED__
#define __IEMSOrbit2_INTERFACE_DEFINED__

/* interface IEMSOrbit2 */
/* [unique][helpstring][uuid][object] */ 

typedef struct _tagEMSTLEDATA2
    {
    EMSTLEDATA tle;
    EMSTIME timeEffective;
    EMSTIME timeModified;
    } 	EMSTLEDATA2;

typedef struct _tagEMSTLEDATA2 *LPEMSTLEDATA2;


EXTERN_C const IID IID_IEMSOrbit2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90EFA3C3-CFD8-4a39-9321-AC2C7C18D5AA")
    IEMSOrbit2 : public IEMSOrbit
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSatelliteTLE2( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSatelliteTLE2( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertOrbitVectorToTLE( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEMSOrbit2Vtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEMSOrbit2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEMSOrbit2 * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEMSOrbit2 * This);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbit)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbit )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbit)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbit )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLE )( 
            IEMSOrbit2 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLE )( 
            IEMSOrbit2 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, SetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [in] */ const EMSTLEDATA *pTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, PropogateTLE)
        HRESULT ( STDMETHODCALLTYPE *PropogateTLE )( 
            IEMSOrbit2 * This,
            /* [in] */ EMSTLEDATA *lpTLEData,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpPropogatedTLE);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetOrbitNumber)
        HRESULT ( STDMETHODCALLTYPE *GetOrbitNumber )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ ULONG *lpulOrbitNumber);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, Reload)
        HRESULT ( STDMETHODCALLTYPE *Reload )( 
            IEMSOrbit2 * This,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, SetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE2 )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, GetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE2 )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, ConvertOrbitVectorToTLE)
        HRESULT ( STDMETHODCALLTYPE *ConvertOrbitVectorToTLE )( 
            IEMSOrbit2 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        END_INTERFACE
    } IEMSOrbit2Vtbl;

    interface IEMSOrbit2
    {
        CONST_VTBL struct IEMSOrbit2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEMSOrbit2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEMSOrbit2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEMSOrbit2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEMSOrbit2_EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit2_InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit2_EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit2_InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit2_SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData) ) 

#define IEMSOrbit2_GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit2_PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE)	\
    ( (This)->lpVtbl -> PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE) ) 

#define IEMSOrbit2_GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber)	\
    ( (This)->lpVtbl -> GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber) ) 

#define IEMSOrbit2_Reload(This,dwFlags)	\
    ( (This)->lpVtbl -> Reload(This,dwFlags) ) 


#define IEMSOrbit2_SetSatelliteTLE2(This,ulSatelliteID,lpTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE2(This,ulSatelliteID,lpTLEData) ) 

#define IEMSOrbit2_GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit2_ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData)	\
    ( (This)->lpVtbl -> ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEMSOrbit2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EMSORBIT_0000_0002 */
/* [local] */ 

typedef /* [unique] */ IEMSOrbit2 *LPEMSORBIT2;



extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0002_v0_0_s_ifspec;

#ifndef __IEMSOrbit3_INTERFACE_DEFINED__
#define __IEMSOrbit3_INTERFACE_DEFINED__

/* interface IEMSOrbit3 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEMSOrbit3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("31D9B372-71A0-4298-9122-F7DFFB067439")
    IEMSOrbit3 : public IEMSOrbit2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSatelliteTLE3( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData,
            /* [in] */ DWORD dwFlags) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEMSOrbit3Vtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEMSOrbit3 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEMSOrbit3 * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEMSOrbit3 * This);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbit)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbit )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbit)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbit )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLE )( 
            IEMSOrbit3 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLE )( 
            IEMSOrbit3 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, SetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [in] */ const EMSTLEDATA *pTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, PropogateTLE)
        HRESULT ( STDMETHODCALLTYPE *PropogateTLE )( 
            IEMSOrbit3 * This,
            /* [in] */ EMSTLEDATA *lpTLEData,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpPropogatedTLE);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetOrbitNumber)
        HRESULT ( STDMETHODCALLTYPE *GetOrbitNumber )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ ULONG *lpulOrbitNumber);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, Reload)
        HRESULT ( STDMETHODCALLTYPE *Reload )( 
            IEMSOrbit3 * This,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, SetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE2 )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, GetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE2 )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, ConvertOrbitVectorToTLE)
        HRESULT ( STDMETHODCALLTYPE *ConvertOrbitVectorToTLE )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit3, SetSatelliteTLE3)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE3 )( 
            IEMSOrbit3 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData,
            /* [in] */ DWORD dwFlags);
        
        END_INTERFACE
    } IEMSOrbit3Vtbl;

    interface IEMSOrbit3
    {
        CONST_VTBL struct IEMSOrbit3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEMSOrbit3_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEMSOrbit3_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEMSOrbit3_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEMSOrbit3_EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit3_InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit3_EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit3_InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit3_SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData) ) 

#define IEMSOrbit3_GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit3_PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE)	\
    ( (This)->lpVtbl -> PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE) ) 

#define IEMSOrbit3_GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber)	\
    ( (This)->lpVtbl -> GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber) ) 

#define IEMSOrbit3_Reload(This,dwFlags)	\
    ( (This)->lpVtbl -> Reload(This,dwFlags) ) 


#define IEMSOrbit3_SetSatelliteTLE2(This,ulSatelliteID,lpTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE2(This,ulSatelliteID,lpTLEData) ) 

#define IEMSOrbit3_GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit3_ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData)	\
    ( (This)->lpVtbl -> ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData) ) 


#define IEMSOrbit3_SetSatelliteTLE3(This,ulSatelliteID,lpTLEData,dwFlags)	\
    ( (This)->lpVtbl -> SetSatelliteTLE3(This,ulSatelliteID,lpTLEData,dwFlags) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEMSOrbit3_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EMSORBIT_0000_0003 */
/* [local] */ 

typedef /* [unique] */ IEMSOrbit3 *LPEMSORBIT3;



extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0003_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0003_v0_0_s_ifspec;

#ifndef __IEMSOrbit4_INTERFACE_DEFINED__
#define __IEMSOrbit4_INTERFACE_DEFINED__

/* interface IEMSOrbit4 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEMSOrbit4;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("738265B6-9E40-44ff-B8A7-E5F681AFBA18")
    IEMSOrbit4 : public IEMSOrbit3
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EarthFixedOrbitDouble( 
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InertialOrbitDouble( 
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EarthFixedOrbitTLEDouble( 
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InertialOrbitTLEDouble( 
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertOrbitVectorToTLEDouble( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORDD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEMSOrbit4Vtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEMSOrbit4 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEMSOrbit4 * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEMSOrbit4 * This);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbit)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbit )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbit)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbit )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLE )( 
            IEMSOrbit4 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLE )( 
            IEMSOrbit4 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, SetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [in] */ const EMSTLEDATA *pTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, PropogateTLE)
        HRESULT ( STDMETHODCALLTYPE *PropogateTLE )( 
            IEMSOrbit4 * This,
            /* [in] */ EMSTLEDATA *lpTLEData,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpPropogatedTLE);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetOrbitNumber)
        HRESULT ( STDMETHODCALLTYPE *GetOrbitNumber )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ ULONG *lpulOrbitNumber);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, Reload)
        HRESULT ( STDMETHODCALLTYPE *Reload )( 
            IEMSOrbit4 * This,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, SetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE2 )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, GetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE2 )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, ConvertOrbitVectorToTLE)
        HRESULT ( STDMETHODCALLTYPE *ConvertOrbitVectorToTLE )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit3, SetSatelliteTLE3)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE3 )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, EarthFixedOrbitDouble)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitDouble )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, InertialOrbitDouble)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitDouble )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, EarthFixedOrbitTLEDouble)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLEDouble )( 
            IEMSOrbit4 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, InertialOrbitTLEDouble)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLEDouble )( 
            IEMSOrbit4 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, ConvertOrbitVectorToTLEDouble)
        HRESULT ( STDMETHODCALLTYPE *ConvertOrbitVectorToTLEDouble )( 
            IEMSOrbit4 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORDD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        END_INTERFACE
    } IEMSOrbit4Vtbl;

    interface IEMSOrbit4
    {
        CONST_VTBL struct IEMSOrbit4Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEMSOrbit4_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEMSOrbit4_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEMSOrbit4_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEMSOrbit4_EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData) ) 

#define IEMSOrbit4_GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit4_PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE)	\
    ( (This)->lpVtbl -> PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE) ) 

#define IEMSOrbit4_GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber)	\
    ( (This)->lpVtbl -> GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber) ) 

#define IEMSOrbit4_Reload(This,dwFlags)	\
    ( (This)->lpVtbl -> Reload(This,dwFlags) ) 


#define IEMSOrbit4_SetSatelliteTLE2(This,ulSatelliteID,lpTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE2(This,ulSatelliteID,lpTLEData) ) 

#define IEMSOrbit4_GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit4_ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData)	\
    ( (This)->lpVtbl -> ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData) ) 


#define IEMSOrbit4_SetSatelliteTLE3(This,ulSatelliteID,lpTLEData,dwFlags)	\
    ( (This)->lpVtbl -> SetSatelliteTLE3(This,ulSatelliteID,lpTLEData,dwFlags) ) 


#define IEMSOrbit4_EarthFixedOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_InertialOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_EarthFixedOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_InertialOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit4_ConvertOrbitVectorToTLEDouble(This,ulSatelliteID,timeCoord,lpTLEData)	\
    ( (This)->lpVtbl -> ConvertOrbitVectorToTLEDouble(This,ulSatelliteID,timeCoord,lpTLEData) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEMSOrbit4_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EMSORBIT_0000_0004 */
/* [local] */ 

typedef /* [unique] */ IEMSOrbit4 *LPEMSORBIT4;



extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0004_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0004_v0_0_s_ifspec;

#ifndef __IEMSOrbit5_INTERFACE_DEFINED__
#define __IEMSOrbit5_INTERFACE_DEFINED__

/* interface IEMSOrbit5 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEMSOrbit5;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DEDFE2BA-F2FB-4bee-9744-207931959F40")
    IEMSOrbit5 : public IEMSOrbit4
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSatelliteTLE3( 
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData,
            /* [out] */ DWORD *lpdwFlags) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEMSOrbit5Vtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEMSOrbit5 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEMSOrbit5 * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEMSOrbit5 * This);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbit)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbit )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbit)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbit )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, EarthFixedOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLE )( 
            IEMSOrbit5 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, InertialOrbitTLE)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLE )( 
            IEMSOrbit5 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, SetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [in] */ const EMSTLEDATA *pTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetSatelliteTLE)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, PropogateTLE)
        HRESULT ( STDMETHODCALLTYPE *PropogateTLE )( 
            IEMSOrbit5 * This,
            /* [in] */ EMSTLEDATA *lpTLEData,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA *lpPropogatedTLE);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, GetOrbitNumber)
        HRESULT ( STDMETHODCALLTYPE *GetOrbitNumber )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ ULONG *lpulOrbitNumber);
        
        DECLSPEC_XFGVIRT(IEMSOrbit, Reload)
        HRESULT ( STDMETHODCALLTYPE *Reload )( 
            IEMSOrbit5 * This,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, SetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE2 )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, GetSatelliteTLE2)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE2 )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit2, ConvertOrbitVectorToTLE)
        HRESULT ( STDMETHODCALLTYPE *ConvertOrbitVectorToTLE )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit3, SetSatelliteTLE3)
        HRESULT ( STDMETHODCALLTYPE *SetSatelliteTLE3 )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTLEDATA2 *lpTLEData,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, EarthFixedOrbitDouble)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitDouble )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, InertialOrbitDouble)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitDouble )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, EarthFixedOrbitTLEDouble)
        HRESULT ( STDMETHODCALLTYPE *EarthFixedOrbitTLEDouble )( 
            IEMSOrbit5 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, InertialOrbitTLEDouble)
        HRESULT ( STDMETHODCALLTYPE *InertialOrbitTLEDouble )( 
            IEMSOrbit5 * This,
            /* [in] */ EMSTLEDATA *lpTLE,
            /* [size_is][in] */ const EMSTIME *pvTime,
            /* [in] */ ULONG cbCount,
            /* [length_is][size_is][out] */ EMSTIMECOORDD *pvCoord,
            /* [out] */ ULONG *pcbResult);
        
        DECLSPEC_XFGVIRT(IEMSOrbit4, ConvertOrbitVectorToTLEDouble)
        HRESULT ( STDMETHODCALLTYPE *ConvertOrbitVectorToTLEDouble )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ EMSTIMECOORDD timeCoord,
            /* [out] */ EMSTLEDATA *lpTLEData);
        
        DECLSPEC_XFGVIRT(IEMSOrbit5, GetSatelliteTLE3)
        HRESULT ( STDMETHODCALLTYPE *GetSatelliteTLE3 )( 
            IEMSOrbit5 * This,
            /* [in] */ ULONG ulSatelliteID,
            /* [in] */ const EMSTIME eTime,
            /* [out] */ EMSTLEDATA2 *lpTLEData,
            /* [out] */ DWORD *lpdwFlags);
        
        END_INTERFACE
    } IEMSOrbit5Vtbl;

    interface IEMSOrbit5
    {
        CONST_VTBL struct IEMSOrbit5Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEMSOrbit5_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEMSOrbit5_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEMSOrbit5_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEMSOrbit5_EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbit(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLE(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE(This,ulSatelliteID,eTime,pTLEData) ) 

#define IEMSOrbit5_GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit5_PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE)	\
    ( (This)->lpVtbl -> PropogateTLE(This,lpTLEData,eTime,lpPropogatedTLE) ) 

#define IEMSOrbit5_GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber)	\
    ( (This)->lpVtbl -> GetOrbitNumber(This,ulSatelliteID,eTime,lpulOrbitNumber) ) 

#define IEMSOrbit5_Reload(This,dwFlags)	\
    ( (This)->lpVtbl -> Reload(This,dwFlags) ) 


#define IEMSOrbit5_SetSatelliteTLE2(This,ulSatelliteID,lpTLEData)	\
    ( (This)->lpVtbl -> SetSatelliteTLE2(This,ulSatelliteID,lpTLEData) ) 

#define IEMSOrbit5_GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData)	\
    ( (This)->lpVtbl -> GetSatelliteTLE2(This,ulSatelliteID,eTime,lpTLEData) ) 

#define IEMSOrbit5_ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData)	\
    ( (This)->lpVtbl -> ConvertOrbitVectorToTLE(This,ulSatelliteID,timeCoord,lpTLEData) ) 


#define IEMSOrbit5_SetSatelliteTLE3(This,ulSatelliteID,lpTLEData,dwFlags)	\
    ( (This)->lpVtbl -> SetSatelliteTLE3(This,ulSatelliteID,lpTLEData,dwFlags) ) 


#define IEMSOrbit5_EarthFixedOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_InertialOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitDouble(This,ulSatelliteID,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_EarthFixedOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> EarthFixedOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_InertialOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult)	\
    ( (This)->lpVtbl -> InertialOrbitTLEDouble(This,lpTLE,pvTime,cbCount,pvCoord,pcbResult) ) 

#define IEMSOrbit5_ConvertOrbitVectorToTLEDouble(This,ulSatelliteID,timeCoord,lpTLEData)	\
    ( (This)->lpVtbl -> ConvertOrbitVectorToTLEDouble(This,ulSatelliteID,timeCoord,lpTLEData) ) 


#define IEMSOrbit5_GetSatelliteTLE3(This,ulSatelliteID,eTime,lpTLEData,lpdwFlags)	\
    ( (This)->lpVtbl -> GetSatelliteTLE3(This,ulSatelliteID,eTime,lpTLEData,lpdwFlags) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEMSOrbit5_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EMSORBIT_0000_0005 */
/* [local] */ 

typedef /* [unique] */ IEMSOrbit5 *LPEMSORBIT5;



extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0005_v0_0_s_ifspec;

#ifndef __IEMSOrbitInit_INTERFACE_DEFINED__
#define __IEMSOrbitInit_INTERFACE_DEFINED__

/* interface IEMSOrbitInit */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEMSOrbitInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CE4EC876-A5CF-4f82-9B7A-5362D6D363F1")
    IEMSOrbitInit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEMSOrbitInitVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEMSOrbitInit * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEMSOrbitInit * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEMSOrbitInit * This);
        
        DECLSPEC_XFGVIRT(IEMSOrbitInit, Initialize)
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IEMSOrbitInit * This);
        
        END_INTERFACE
    } IEMSOrbitInitVtbl;

    interface IEMSOrbitInit
    {
        CONST_VTBL struct IEMSOrbitInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEMSOrbitInit_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEMSOrbitInit_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEMSOrbitInit_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEMSOrbitInit_Initialize(This)	\
    ( (This)->lpVtbl -> Initialize(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEMSOrbitInit_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EMSORBIT_0000_0006 */
/* [local] */ 

typedef /* [unique] */ IEMSOrbitInit *LPEMSORBITINIT;



extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0006_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EMSORBIT_0000_0006_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


