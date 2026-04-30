/* DTrackSDK Python Wrapper: DTrackSDK-pybind.cpp
 *
 * Data type definitions.
 *
 * Copyright (c) 2025-2026 Advanced Realtime Tracking GmbH & Co. KG
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <functional>
#include <type_traits>
#include <array>
#include <sstream>

#include "DTrackSDK.hpp"


//-------------------------------------------------------------

namespace py = pybind11;
using namespace pybind11::literals;

using DTrackMarker = DTrackSDK_Datatypes::DTrackMarker;
using DTrackBody = DTrackSDK_Datatypes::DTrackBody;
using DTrackQuaternion = DTrackSDK_Datatypes::DTrackQuaternion;
using DTrackFlyStick = DTrackSDK_Datatypes::DTrackFlyStick;
using DTrackFinger = DTrackSDK_Datatypes::DTrackHand::DTrackFinger;
using DTrackHand = DTrackSDK_Datatypes::DTrackHand;


//-------------------------------------------------------------
// Expose algebra-related c-style arrays of `DTrackSDK_Datatypes`
//  (like 'loc', 'rot') as read-writable numpy arrays. Access to the
//  underlying c-style array by numpy is column-major, corresponding
//  to ART convention, in contrast to numpy's default order (see e.g.
//  "DTRACK3_Programmers-Guide" sec. 3.5 "6DOF Standard Body").
//-------------------------------------------------------------


// Matrix as numpy array of shape '(Dim0,Dim1)'
template< typename DTrackT, pybind11::ssize_t Dim0, pybind11::ssize_t Dim1 = Dim0 >
struct MatrixProperty
{
    // `def_property` by default uses `return_value_policy::reference_internal`,
    // which includes `keep_alive<0,1>`. Only relevant for the `getter`.

    static py::array_t<double>  getter( DTrackT& dtType, double darr[Dim0*Dim1] )
    { 
        return py::array_t<double>(
            { Dim0, Dim1 },
            { sizeof(double), Dim0 * sizeof(double) }, // Column-major strides
            darr,
            py::cast( dtType )
        );
    };


    static void  setter(const py::array_t<double>& nparr, double darr[Dim0 * Dim1])
    {
        // Force cast of incoming numpy arrays (or even python lists), 
        // no matter if already "f_style double" or not
        // TODO, cast only if necessary

        const pybind11::ssize_t ndim = 2;

        using array_double_fstyle = py::array_t< double, py::array::f_style | py::array::forcecast >;
        const auto nparr_ = nparr.cast< array_double_fstyle >();

        py::buffer_info buf = nparr_.request();

        if (buf.ndim != ndim || buf.shape[0] != Dim0 || buf.shape[1] != Dim1)
        {
            std::stringstream s;
            s << "Invalid shape (" << buf.shape[0] << "," << buf.shape[1] << ")"
                << ", expected (" << Dim0 << "," << Dim1 << ")";
            throw std::runtime_error(s.str());
        }

        std::memcpy(darr, buf.ptr, Dim0 * Dim1 * sizeof(double));
    }
};


// Vector as numpy array of shape '(Dim,1)' (not as '(Dim,)')
template< typename DTrackT, pybind11::ssize_t Dim >
struct VectorProperty : MatrixProperty< DTrackT, Dim, 1 >
{};



//-------------------------------------------------------------
// Expose non-algebraic c-style arrays of DTrackSDK_Datatypes as lists, implying copies
//-------------------------------------------------------------

template< typename ValueT, std::size_t Length >
struct ListProperty
{
    static std::array< ValueT, Length >  setter( const ValueT field[Length] )
    {
        std::array< ValueT, Length > arr;
        std::copy( field, field + Length, arr.begin() );
        return arr;
    }
};


//-------------------------------------------------------------
// Proxy class to expose non-algebraic c-style arrays as lists, avoiding copies
//-------------------------------------------------------------

template< typename ArrayT >
struct ListPropertyProxy
{
    static_assert( std::is_array<ArrayT>::value, "ArrayT is not an array type");
    using SizeType  = typename pybind11::ssize_t;
    using ArrayType = ArrayT;
    using ValueType = typename std::remove_all_extents< ArrayT >::type;
    static constexpr SizeType SIZE = std::extent< ArrayT >::value;
    
    ListPropertyProxy( ArrayType& array )
        : m_array( array )
    {}

    ValueType& getitem( SizeType index )
    {
        if (index >= SIZE) throw py::index_error();
        return m_array[index];
    }

    static SizeType len()
    { return SIZE; }

protected:
    ArrayType& m_array;
};


template< typename ArrayT >
void declareListProperty( pybind11::module_& handle, const char* name )
{
    using Proxy = ListPropertyProxy< ArrayT >;
    py::class_< Proxy > proxy(handle, name);

    proxy.def( "__getitem__",
        [](Proxy& self, typename Proxy::SizeType index)
        { return self.getitem(index); }
    );

    proxy.def("__len__", &Proxy::len);
}


//-------------------------------------------------------------
// Instead of a class 'DTrackQuaternion' we expose a numpy 
// array of shape '(4,)' with real part 'w' at first position
//-------------------------------------------------------------

py::array_t<double> quaternion2array( const DTrackQuaternion& q )
{
    // Rely on layout w,x,y,z in DTrackQuaternion
    return py::array_t<double>( {4}, {sizeof(double)}, &q.w );
}



//-------------------------------------------------------------

PYBIND11_MODULE( DTrackSDK, handle )
{
    handle.doc() = "DTrack SDK main class";
    handle.attr("__version__") = DTrackSDK::getVersion();


    //-------------------------------------------------------------
    // rotmatrix2quaternion
    //-------------------------------------------------------------

    handle.def( "rotmatrix2quaternion", 
        []( const py::array_t<double>& nparr )
        {
            const pybind11::ssize_t ndim = 2;
            
            // TODO, cast if necessary
            using array_double_fstyle = py::array_t< double, py::array::f_style | py::array::forcecast >;
            const auto nparr_ = nparr.cast< array_double_fstyle >();
            
            py::buffer_info buf = nparr_.request();
            
            if (buf.ndim != ndim || buf.shape[0] != 3 || buf.shape[1] != 3)
            {
                std::stringstream s;
                s << "Invalid shape (" << buf.shape[0] << "," << buf.shape[1] << ")"
                    << ", expected (3,3)";
                throw std::runtime_error(s.str());
            }
            
            DTrackQuaternion q = DTrackSDK_Datatypes::rot2quat( static_cast<double*>( buf.ptr ) );
            return quaternion2array(q);
        
        }, 
        py::return_value_policy::reference
    );



    //-------------------------------------------------------------
    // DTrackMarker
    //-------------------------------------------------------------

    py::class_< DTrackMarker > dTrackMarker( handle, "DTrackMarker" );

    dTrackMarker.def( py::init<>() );

    dTrackMarker.def_readonly( "id", &DTrackMarker::id );
    dTrackMarker.def_readonly( "quality", &DTrackMarker::quality );

    dTrackMarker.def_property( "loc",
        []( DTrackMarker& m ) {  
            return VectorProperty< DTrackMarker, 3 >::getter( m, m.loc ); },
        []( DTrackMarker& m, const py::array_t<double>& arr ) {  
            VectorProperty< DTrackMarker, 3 >::setter( arr, m.loc ); }
    );


    //-------------------------------------------------------------
    // DTrackBody
    //-------------------------------------------------------------

    py::class_< DTrackBody > dTrackBody( handle, "DTrackBody");
    dTrackBody.def( py::init<>() );

    dTrackBody.def_readonly( "id",         &DTrackBody::id );
    dTrackBody.def_readonly( "quality",    &DTrackBody::quality );


    dTrackBody.def_property( "loc",
        []( DTrackBody& x ) {  
            return VectorProperty< DTrackBody, 3 >::getter( x, x.loc ); },
        []( DTrackBody& x, const py::array_t<double>& arr ) {  
            VectorProperty< DTrackBody, 3 >::setter( arr, x.loc ); }
    );

    dTrackBody.def_property( "rot",
        []( DTrackBody& x ) {  
            return MatrixProperty< DTrackBody, 3 >::getter( x, x.rot ); },
        []( DTrackBody& x, const py::array_t<double>& arr ) {  
            MatrixProperty< DTrackBody, 3 >::setter( arr, x.rot ); }
    );

    dTrackBody.def_property( "covref",
        []( DTrackBody& x ) {  
            return VectorProperty< DTrackBody, 3 >::getter( x, x.covref ); },
        []( DTrackBody& x, const py::array_t<double>& arr ) {  
            VectorProperty< DTrackBody, 3 >::setter( arr, x.covref ); },

        "Covariance reference point [mm]. "
        "See D. Pustka et al., Determining the point of minimum error for 6DOF pose uncertainty representation, "
        "2010 IEEE International Symposium on Mixed and Augmented Reality, "
        "Seoul, 2010, pp. 37-45."
    );

    dTrackBody.def_property( "cov",
        []( DTrackBody& x ) {  
            return MatrixProperty< DTrackBody, 6 >::getter( x, x.cov ); },
        []( DTrackBody& x, py::array_t<double> arr ) {  
            MatrixProperty< DTrackBody, 6 >::setter( arr, x.cov ); }
    );


    dTrackBody.def( "isTracked", &DTrackBody::isTracked );

    dTrackBody.def( "getQuaternion", []( DTrackBody &x )
    { return quaternion2array( x.getQuaternion() ); });



    //-------------------------------------------------------------
    // DTrackFlyStick
    //-------------------------------------------------------------

    py::class_< DTrackFlyStick > dTrackFlystick( handle, "DTrackFlyStick");
    dTrackFlystick.def( py::init<>() );

    dTrackFlystick.def_readonly( "id",             &DTrackFlyStick::id );
    dTrackFlystick.def_readonly( "quality",        &DTrackFlyStick::quality );
    dTrackFlystick.def_readonly( "num_button",     &DTrackFlyStick::num_button );
    dTrackFlystick.def_readonly( "num_joystick",   &DTrackFlyStick::num_joystick );


    dTrackFlystick.def_property( "loc",
        []( DTrackFlyStick& x ) {  
            return VectorProperty< DTrackFlyStick, 3 >::getter( x, x.loc ); },
        []( DTrackFlyStick& x, const py::array_t<double>& arr ) {  
            VectorProperty< DTrackFlyStick, 3 >::setter( arr, x.loc ); }
    );

    dTrackFlystick.def_property( "rot",
        []( DTrackFlyStick& x ) {  
            return MatrixProperty< DTrackFlyStick, 3 >::getter( x, x.rot ); },
        []( DTrackFlyStick& x, const py::array_t<double>& arr ) {  
            MatrixProperty< DTrackFlyStick, 3 >::setter( arr, x.rot ); }
    );


    dTrackFlystick.def_property_readonly( "button",
        []( const DTrackFlyStick &x ) {
            return ListProperty< int, DTRACKSDK_FLYSTICK_MAX_BUTTON >::setter( x.button ); }
    );

    dTrackFlystick.def_property_readonly( "joystick",
        []( const DTrackFlyStick &x ) {
            return ListProperty< double, DTRACKSDK_FLYSTICK_MAX_JOYSTICK >::setter( x.joystick ); } 
    );


    dTrackFlystick.def( "isTracked", &DTrackFlyStick::isTracked );

    dTrackFlystick.def( "getQuaternion", []( DTrackFlyStick &x )
    { return quaternion2array( x.getQuaternion() ); });



    //-------------------------------------------------------------
    // DTrackFinger
    //-------------------------------------------------------------

    py::class_< DTrackFinger > dTrackFinger( handle, "DTrackFinger");
    dTrackFinger.def( py::init<>() );


    dTrackFinger.def_readonly( "radiustip",  &DTrackFinger::radiustip );


    dTrackFinger.def_property( "loc",
        []( DTrackFinger& x ) {  
            return VectorProperty< DTrackFinger, 3 >::getter( x, x.loc ); },
        []( DTrackFinger& x, const py::array_t<double>& arr ) {  
            VectorProperty< DTrackFinger, 3 >::setter( arr, x.loc ); }
    );

    dTrackFinger.def_property( "rot",
        []( DTrackFinger& x ) {  
            return MatrixProperty< DTrackFinger, 3 >::getter( x, x.rot ); },
        []( DTrackFinger& x, const py::array_t<double>& arr ) {  
            MatrixProperty< DTrackFinger, 3 >::setter( arr, x.rot ); }
    );


    dTrackFinger.def_property_readonly( "lengthphalanx",
        []( const DTrackFinger &x )
        {
            constexpr std::size_t  n  = sizeof( DTrackFinger::lengthphalanx ) / sizeof( double );
            return ListProperty< double, n >::setter( x.lengthphalanx ); 
        }
    );

    dTrackFinger.def_property_readonly( "anglephalanx",
        []( const DTrackFinger &x ) 
        {
            constexpr std::size_t  n  = sizeof( DTrackFinger::anglephalanx ) / sizeof( double );
            return ListProperty< double, n >::setter( x.anglephalanx ); 
        }
    );


    dTrackFinger.def( "getQuaternion", []( DTrackFinger &x )
    { return quaternion2array( x.getQuaternion() ); });



    //-------------------------------------------------------------
    // DTrackHand
    //-------------------------------------------------------------

    py::class_< DTrackHand > dTrackHand( handle, "DTrackHand");
    dTrackHand.def( py::init<>() );

    dTrackHand.def_readonly( "id",             &DTrackHand::id );
    dTrackHand.def_readonly( "quality",        &DTrackHand::quality );
    dTrackHand.def_readonly( "lr",             &DTrackHand::lr );
    dTrackHand.def_readonly( "nfinger",        &DTrackHand::nfinger );


    dTrackHand.def_property( "loc",
        []( DTrackHand& x ) {  
            return VectorProperty< DTrackHand, 3 >::getter( x, x.loc ); },
        []( DTrackHand& x, const py::array_t<double>& arr ) {  
            VectorProperty< DTrackHand, 3 >::setter( arr, x.loc ); }
    );

    dTrackHand.def_property( "rot",
        []( DTrackHand& x ) {  
            return MatrixProperty< DTrackHand, 3 >::getter( x, x.rot ); },
        []( DTrackHand& x, const py::array_t<double>& arr ) {  
            MatrixProperty< DTrackHand, 3 >::setter( arr, x.rot ); }
    );


    using DTrackFingerArray = DTrackFinger[DTRACKSDK_HAND_MAX_FINGER];
    declareListProperty< DTrackFingerArray >( handle, "FingerListProperty");
    dTrackHand.def_property_readonly( "finger",
        [](DTrackHand& hand)
        { return ListPropertyProxy< DTrackFingerArray >( hand.finger); }
    );

    dTrackHand.def( "isTracked", &DTrackHand::isTracked );

    dTrackHand.def( "getQuaternion", []( DTrackHand &x )
    { return quaternion2array( x.getQuaternion() ); });



    //-------------------------------------------------------------
    // DTrackSDK
    //-------------------------------------------------------------

    py::class_<DTrackSDK> dtrack( handle, "dtrack" );

    dtrack.def( py::init< int >() );        // TODO, unsigned

    dtrack.def( "setDataTimeoutUS",         &DTrackSDK::setDataTimeoutUS );
    dtrack.def( "isDataInterfaceValid",     &DTrackSDK::isDataInterfaceValid );
    dtrack.def( "getFrameCounter",          &DTrackSDK::getFrameCounter );
    dtrack.def( "getTimeStamp",             &DTrackSDK::getTimeStamp );
    dtrack.def( "processPacket",            &DTrackSDK::processPacket );
    dtrack.def( "getNumBody",               &DTrackSDK::getNumBody );
    dtrack.def( "getNumFlyStick",           &DTrackSDK::getNumFlyStick );
    dtrack.def( "getNumHand",               &DTrackSDK::getNumHand );

    // Example packet for `processPacket` in python:
    // "fr 4460\nts 38695.120802\nts2 1738838695 120802 005450\n6d 1 [0 1.000][-1309.374 -242.666 696.194 -124.0391 84.1737 -151.4170][-0.089140 0.991710 -0.092544 0.048567 0.097131 0.994086 0.994834 0.084119 -0.056822]"
    dtrack.def( "processPacket",            &DTrackSDK::processPacket );


    // Default value `DTrackSDK::DTRACK2_PORT_UDPSENDER` is not accessible
    dtrack.def( "enableStatefulFirewallConnection", 
        []( DTrackSDK& self, const std::string& senderHost) {
            return self.enableStatefulFirewallConnection( senderHost ); },
        py::arg( "senderHost") );

    dtrack.def( "enableStatefulFirewallConnection", 
        []( DTrackSDK& self, const std::string& senderHost, unsigned short senderPort ) {
            return self.enableStatefulFirewallConnection( senderHost, senderPort ); },
        py::arg( "senderHost"), py::arg( "senderPort") );


    dtrack.def( "receive", &DTrackSDK::receive,
        "Update internal data structures.");

    dtrack.def( "getBody", &DTrackSDK::getBody, py::return_value_policy::reference,
	    "Expose a reference to an internal DTrackBody data structure. "
	    "An obtained reference is potentially invalid after a new call to DTrackSDK::receive().",
	    py::arg("id") );

    dtrack.def( "getFlyStick", &DTrackSDK::getFlyStick, py::return_value_policy::reference,
        "Expose a reference to an internal DTrackFlystick data structure. "
        "An obtained reference is potentially invalid after a new call to DTrackSDK::receive().",
        py::arg("id") );

    dtrack.def( "getHand", &DTrackSDK::getHand, py::return_value_policy::reference,
        "Expose a reference to an internal DTrackHand data structure. "
        "An obtained reference is potentially invalid after a new call to DTrackSDK::receive().",
        py::arg("id") );

}


