/*
 *
 *  Sample Surface Plugin - Fabric Engine
 *
 *  This sample code demonstrates how to implement a basic procedural surface.
 *  This is a little more convoluted than the standard procedural item, as we're
 *  also reading user channels on the item to generate the surface. At the moment,
 *  we don't do anything with these user channels, but we could use them as inputs
 *  to Fabric Engine to evaluate the procedural geometry.
 *
 *  NOTE: Anything that needs modifying to work with Fabric Engine has been marked
 *  with "FETODO". Search the code for this and it should be pretty obvious where
 *  to insert your code.
 *
 */

#include <lxidef.h>

#include <lx_channelui.hpp>
#include <lx_draw.hpp>
#include <lx_item.hpp>
#include <lx_listener.hpp>
#include <lx_mesh.hpp>
#include <lx_package.hpp>
#include <lx_plugin.hpp>
#include <lx_surface.hpp>
#include <lx_tableau.hpp>
#include <lx_vertex.hpp>
#include <lx_vmodel.hpp>

#include <lxu_math.hpp>
#include <lxu_modifier.hpp>

#include "lxu_log.hpp"
#include "lxlog.h"

#include <map>
#include <iterator>
#include <vector>
#include <algorithm>

/*
 *  Define the server and channel names.
 */

#define SERVER_NAME     "surface.sample.fix"

#define CHAN_INSTOBJ        "instObj"
#define CHAN_DIMENSIONS     "dimensions"

static CLxItemType   gItemTypeFix (SERVER_NAME);

/*
 *  Disambiguate with a namespace.
 */

namespace Surface_Sample_Fix
{
  // log system.
  #define LOG_SYSTEM_NAME "surface.sample.fix.log"
  class CItemLog : public CLxLogMessage
  {
   public:
      CItemLog() : CLxLogMessage(LOG_SYSTEM_NAME) { }
      const char *GetFormat()     { return "n.a."; }
      const char *GetVersion()    { return "n.a."; }
      const char *GetCopyright()  { return "n.a."; }
  };
  CItemLog gLog;
  void feLog(void *userData, const char *s, unsigned int length)
  {
    const char *p = (s != NULL ? s : "s == NULL");
    gLog.Message(LXe_INFO, "[SURF.SAMPLE.FIX]", p, " ");
  }
  void feLog(void *userData, const std::string &s)
  {
    feLog(userData, s.c_str(), s.length());
  }
  void feLog(const std::string &s)
  {
    feLog(NULL, s.c_str(), s.length());
  }
  void feLogError(void *userData, const char *s, unsigned int length)
  {
    const char *p = (s != NULL ? s : "s == NULL");
    gLog.Message(LXe_FAILED, "[SURF.SAMPLE.FIX ERROR]", p, " ");
  }
  void feLogError(void *userData, const std::string &s)
  {
    feLogError(userData, s.c_str(), s.length());
  }
  void feLogError(const std::string &s)
  {
    feLogError(NULL, s.c_str(), s.length());
  }
  void feLogDebug(void *userData, const char *s, unsigned int length)
  {
    feLog(userData, s, length);
  }
  void feLogDebug(void *userData, const std::string &s)
  {
    feLog(userData, s);
  }
  void feLogDebug(const std::string &s)
  {
    feLog(s);
  }
  void feLogDebug(const std::string &s, int number)
  {
    char t[64];
    sprintf(t, " number = %ld", number);
    feLog(s + t);
  }



/*
 *  First we define a structure that can be used to store a single user channel. It
 *  just stores the channel index and any evaluation index that has been allocated for
 *  this item.
 */

struct ChannelDef
{
    int          chan_index;
    int          eval_index;

    /* [EM] this is done differently (or not yet included) in CanvasPI. */
    CLxUser_Value        chan_value;
    
    ChannelDef () : chan_index (-1), eval_index (-1) {}
};

void userChannels_collect (CLxUser_Item &item, std::vector <ChannelDef> &userChannels)
{
    /*
     *  This function collects all of the user channels on the specified item
     *  and adds them to the provided vector of channel definitions. We loop
     *  through all channels on the item and test their package. If they have
     *  no package, then it's a user channel and it's added to the vector.
     *  We also check if the channel type is a divider, if it is, we skip it.
     */
    
    unsigned         count = 0;
    
    userChannels.clear ();
    
    if (!item.test ())
        return;
    
    item.ChannelCount (&count);
    
    for (unsigned i = 0; i < count; i++)
    {
        const char      *package = NULL, *channel_type = NULL;
    
        if (LXx_OK (item.ChannelPackage (i, &package)) || package)
            continue;
        
        if (LXx_OK (item.ChannelEvalType (i, &channel_type)) && channel_type)
        {
            if (strcmp (channel_type, LXsTYPE_NONE) != 0)
            {
                ChannelDef       channel;
            
                channel.chan_index = i;
            
                userChannels.push_back (channel);
            }
        }
    }
}

/*
 *  We define a class here that defines the channels used to evaluate our surface.
 *  It has some helper functions for reading the channels, but ultimately is just
 *  an object that can be passed around between the modifier and the surface.
 */
 
class SurfDef
{
    public:
        SurfDef () : _size (0.5) {}
    
        LxResult     Prepare        (CLxUser_Evaluation &eval, CLxUser_Item &item, unsigned *index);
        LxResult     Evaluate       (CLxUser_Attributes &attr, unsigned index);
        LxResult     Copy           (SurfDef *other);
        int      Compare        (SurfDef *other);
    
        double           _size;
        std::vector <ChannelDef> _user_channels;
};

LxResult SurfDef::Prepare (CLxUser_Evaluation &eval, CLxUser_Item &item, unsigned *index)
{
    /*
     *  This function is used to add channels and user channels to a modifier.
     *  The modifier will then call the evaluate function and read the channel
     *  values.
     */
    
    if (!eval.test () || !item.test ())
        return LXe_NOINTERFACE;
    
    if (!index)
        return LXe_INVALIDARG;
    
    /*
     *  Collect the user channels on this item.
     */

    _user_channels.clear ();
    userChannels_collect (item, _user_channels);
    
    /*
     *  Allocate any standard channels as inputs.
     */
    
    index[0] = eval.AddChan (item, CHAN_DIMENSIONS, LXfECHAN_READ);
    
    /*
     *  Enumerate over the user channels and add them as inputs to a modifier.
     */
    
    for (unsigned i = 0; i < _user_channels.size (); i++)
    {
        ChannelDef      *channel = &_user_channels[i];
        
        channel->eval_index = eval.AddChan (item, channel->chan_index, LXfECHAN_READ);
    }
    
    return LXe_OK;
}

LxResult SurfDef::Evaluate (CLxUser_Attributes &attr, unsigned index)
{
    /*
     *  Once the channels have been allocated as inputs for the modifier, we'll
     *  evaluate them and store their values.
     */
    
    if (!attr.test ())
        return LXe_NOINTERFACE;
    
    /*
     *  Read the size channel and cache it.
     */
    
    _size = attr.Float (index);
    
    /*
     *  Enumerate over the user channels and read them here. These are just cached
     *  in the ChannelDef object.
     */

    feLogDebug("SurfDef::Evaluate() - num_user_channels -", _user_channels.size ());

/* [EM] this is done differently (or not yet included) in CanvasPI. */
    for (int i = 0; i < _user_channels.size (); i++)
    {
        ChannelDef      *channel = &_user_channels[i];
        
        if (channel->eval_index < 0)
            continue;

        attr.ObjectRO ((unsigned) channel->eval_index, channel->chan_value);
    }
    
    return LXe_OK;
}

LxResult SurfDef::Copy (SurfDef *other)
{
    /*
     *  This function is used to copy the cached channel values from one
     *  surface definition to another. We also copy the cached user channels.
     */
    
    if (!other)
        return LXe_INVALIDARG;
    
    /*
     *  Copy the cached user channel information.
     */
    
    _user_channels.clear ();
    _user_channels.reserve (other->_user_channels.size ());
    std::copy (other->_user_channels.begin (), other->_user_channels.end (), std::back_inserter (_user_channels));
    
    /*
     *  Copy any any built in channel values.
     */
    
    _size = other->_size;
    
    return LXe_OK;
}

int SurfDef::Compare (SurfDef *other)
{
    /*
     *  This function does a comparison of another SurfDef with this one. It
     *  should work like strcmp and return 0 for identical, or -1/1 to imply
     *  relative positioning. For now, we just compare the size channel.
     *
     *  FETODO: Add comparison for user channel values here. This could be
     *  handled by calling Compare on the channel value interfaces.
     */
    
    if (!other)
        return 0;
    
    return (_size > other->_size) - (other->_size > _size);
}

/*
 *  The SurfElement represents a binned surface. A binned surface is essentially a
 *  collection of triangles, all sharing the same material tag. The procedural
 *  surface could be constructed from multiple surface elements or in the simplest
 *  of cases, a single element. A StringTag interface allows the material tag to
 *  easily be queried.
 */

class SurfElement : public CLxImpl_TableauSurface, public CLxImpl_StringTag
{
    public:
        static void initialize ()
        {
            CLxGenericPolymorph *srv = NULL;

            srv = new CLxPolymorph                          <SurfElement>;
            srv->AddInterface       (new CLxIfc_TableauSurface      <SurfElement>);
            srv->AddInterface       (new CLxIfc_StringTag           <SurfElement>);

            lx::AddSpawner          (SERVER_NAME".elmt", srv);
        }
    
        unsigned int     tsrf_FeatureCount  (LXtID4 type)                               LXx_OVERRIDE;
        LxResult     tsrf_FeatureByIndex    (LXtID4 type, unsigned int index, const char **name)            LXx_OVERRIDE;
        LxResult     tsrf_Bound     (LXtTableauBox bbox)                            LXx_OVERRIDE;
        LxResult     tsrf_SetVertex     (ILxUnknownID vdesc_obj)                        LXx_OVERRIDE;
        LxResult     tsrf_Sample        (const LXtTableauBox bbox, float scale, ILxUnknownID trisoup_obj)   LXx_OVERRIDE;
    
        LxResult     stag_Get       (LXtID4 type, const char **tag)                     LXx_OVERRIDE;

        SurfDef     *Definition     ();
    
    private:
        int          _offsets[4];
    
        SurfDef          _surf_def;
};

unsigned int SurfElement::tsrf_FeatureCount (LXtID4 type)
{
    /*
     *  We only define the required features on our surface. We could return
     *  things like UVs or weight maps if we have them, but we'll just assume
     *  the standard set of 4 required features.
     */
    
    return (type == LXiTBLX_BASEFEATURE ? 4 : 0);
}

LxResult SurfElement::tsrf_FeatureByIndex (LXtID4 type, unsigned int index, const char **name)
{
    /*
     *  There are four features that are required; position, object position,
     *  normal and velocity. We could also return any extras if we wanted,
     *  but we must provide these.
     */

    if (type != LXiTBLX_BASEFEATURE)
        return LXe_NOTFOUND;

    switch (index)
    {
        case 0:
            name[0] = LXsTBLX_FEATURE_POS;
            break;

        case 1:
            name[0] = LXsTBLX_FEATURE_OBJPOS;
            break;
        
        case 2:
            name[0] = LXsTBLX_FEATURE_NORMAL;
            break;

        case 3:
            name[0] = LXsTBLX_FEATURE_VEL;
            break;
            
        default:
            return LXe_OUTOFBOUNDS;
    }
    
    return LXe_OK;
}

LxResult SurfElement::tsrf_Bound (LXtTableauBox bbox)
{
    /*
     *  This is expected to return a bounding box for the current binned
     *  element. As we only have one element in the sample, and the element
     *  contains a single polygon plane, we'll just return a bounding box that
     *  encapsulates the plane.
     *
     *  FETODO: Add code here for implementing correct bounding box for
     *  Fabric Engine element.
     */
    
    CLxBoundingBox       bounds;
    
    bounds.clear ();
    
    bounds.add (0.0, 0.0, 0.0);
    bounds.inflate (_surf_def._size);
    
    bounds.getBox6 (bbox);
    
    return LXe_OK;
}

LxResult SurfElement::tsrf_SetVertex (ILxUnknownID vdesc_obj)
{
    /*
     *  When we write points into the triangle soup, we write arbitrary
     *  features into an array. The offset for each feature in the array can
     *  be queried at this point and cached for use in our Sample function.
     *
     *  FETODO: If you add any more vertex features, such as UVs, this
     *  function will need modifying.
     */

    CLxUser_TableauVertex    vertex;
    const char      *name = NULL;
    unsigned         offset = 0;

    if (!vertex.set (vdesc_obj))
        return LXe_NOINTERFACE;

    for (int i = 0; i < 4; i++)
    {
        tsrf_FeatureByIndex (LXiTBLX_BASEFEATURE, i, &name);
        
        if (LXx_OK (vertex.Lookup (LXiTBLX_BASEFEATURE, name, &offset)))
            _offsets[i] = offset;
        else
            _offsets[i] = -1;
    }

    return LXe_OK;
}

LxResult SurfElement::tsrf_Sample (const LXtTableauBox bbox, float scale, ILxUnknownID trisoup_obj)
{
    /*
     *  The Sample function is used to generate the geometry for this Surface
     *  Element. We basically just insert points directly into the triangle
     *  soup feature array and then build polygons/triangles from points at
     *  specific positions in the array.
     *
     *  FETODO: This function will need changing to actually generate
     *  geometry matching the output from Fabric Engine.
     */
    
    CLxUser_TriangleSoup     soup (trisoup_obj);
    LXtTableauBox        bounds;
    LXtFVector       normal, zero;
    LxResult         result = LXe_OK;
    unsigned         index = 0;
    float            features[4 * 3];
    float            positions[4][3] = {{-1.0, 0.0, -1.0}, {1.0, 0.0, -1.0}, {1.0, 0.0, 1.0}, {-1.0, 0.0, 1.0}};
    
    if (!soup.test ())
        return LXe_NOINTERFACE;
    
    /*
     *  Test if the current element is visible. If it isn't, we'll return to
     *  save on evaluation time.
     */
    
    if (LXx_OK (tsrf_Bound (bounds)))
    {
        if (!soup.TestBox (bounds))
            return LXe_OK;
    }
    
    /*
     *  We're only generating triangles/polygons in a single segment. If
     *  something else is being requested, we'll early out.
     */
    
    if (LXx_FAIL (soup.Segment (1, LXiTBLX_SEG_TRIANGLE)))
        return LXe_OK;
    
    /*
     *  Build the geometry. We're creating a simple polygon plane here, so
     *  we'll begin by adding four points to the Triangle Soup and then
     *  calling the Quad function to create a polygon from them. The size
     *  of the plane is dictated by our cached channel on our SurfDef object.
     *  As we're simply entering our points into an array, we have to insert
     *  values for each of the features we intend to set, offset using the
     *  offset value cached in the SetVertex function.
     */
    
    /*
     *  Add the four points to the triangle soup.
     */
    
    LXx_VSET3 (normal, 0.0, 1.0, 0.0);
    LXx_VCLR  (zero);
    
    for (int i = 0; i < 4; i++)
    {
        /*
         *  Position.
         */
    
        LXx_VSCL3 (features + _offsets[0], positions[i], _surf_def._size);
        
        /*
         *  Object Position.
         */
        
        LXx_VCPY  (features + _offsets[1], features + _offsets[0]);
        
        /*
         *  Normal.
         */
        
        LXx_VCPY  (features + _offsets[2], normal);
        
        /*
         *  Velocity.
         */

        LXx_VCPY  (features + _offsets[3], features + _offsets[2]);
        
        /*
         *  Add the array of features to the triangle soup to define the
         *  point.
         */
        
        result = soup.Vertex (features, &index);
        
        if (LXx_FAIL (result))
            break;
    }
    
    /*
     *  Build a quad from the four points added to the triangle soup.
     */
    
    if (LXx_OK (result))
        result = soup.Quad (3, 2, 1, 0);
    
    return result;
}
    
LxResult SurfElement::stag_Get (LXtID4 type, const char **tag)
{
    /*
     *  This function is called to get the polygon tag for all polygons inside
     *  of the bin. We only care about setting the material tag and part tag,
     *  and we'll set them both to default.
     *  
     *  FETODO: If you have some way of defining material tagging through
     *  fabric engine. You'll want to set the tag here so it can be used for
     *  texturing.
     */
    
    if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
    {
        tag[0] = "Default";

        return LXe_OK;
    }
    
    return LXe_NOTFOUND;
}

SurfDef *SurfElement::Definition ()
{
    /*
     *  Return a pointer to our surface definition.
     */

    return &_surf_def;
}

/*
 *  The surface itself represents the entire 3D surface. It is composed of
 *  surface elements, divided up in to bins, based on their material tagging.
 *  It also has a couple of functions for getting things like the bounding box
 *  and GL triangle count.
 */

class Surface : public CLxImpl_Surface
{
    public:
        static void initialize ()
        {
            CLxGenericPolymorph *srv = NULL;

            srv = new CLxPolymorph                          <Surface>;
            srv->AddInterface       (new CLxIfc_Surface         <Surface>);

            lx::AddSpawner          (SERVER_NAME".surf", srv);
        }
    
        LxResult     surf_GetBBox       (LXtBBox *bbox)                         LXx_OVERRIDE;
        LxResult     surf_FrontBBox     (const LXtVector pos, const LXtVector dir, LXtBBox *bbox)   LXx_OVERRIDE;
        LxResult     surf_BinCount      (unsigned int *count)                       LXx_OVERRIDE;
        LxResult     surf_BinByIndex    (unsigned int index, void **ppvObj)             LXx_OVERRIDE;
        LxResult     surf_TagCount      (LXtID4 type, unsigned int *count)              LXx_OVERRIDE;
        LxResult     surf_TagByIndex    (LXtID4 type, unsigned int index, const char **stag)        LXx_OVERRIDE;
        LxResult     surf_GLCount       (unsigned int *count)                       LXx_OVERRIDE;
    
        SurfDef     *Definition     ();
    
    private:
        SurfDef          _surf_def;
};

LxResult Surface::surf_GetBBox (LXtBBox *bbox)
{
    /*
     *  This is expected to return a bounding box for the entire surface.
     *  We just return a uniform box the expanded by our size channel, this
     *  isn't exactly correct, but it'll do for this sample.
     *
     *  FETODO: Add code here for implementing correct bounding box for
     *  Fabric Engine surface.
     */
    
    CLxBoundingBox       bounds;
    
    bounds.clear ();
    
    bounds.add (0.0, 0.0, 0.0);
    bounds.inflate (_surf_def._size);
    
    bounds.get (bbox);
    
    return LXe_OK;
}

LxResult Surface::surf_FrontBBox (const LXtVector pos, const LXtVector dir, LXtBBox *bbox)
{
    /*
     *  FrontBBox is called to get the bounding box for a particular raycast.
     *  For simplicity, we'll fall through to the GetBBox function.
     */
    
    return surf_GetBBox (bbox);
}

LxResult Surface::surf_BinCount (unsigned int *count)
{
    /*
     *  Surface elements are divided into bins, where each bin is a collection
     *  of triangles with the same polygon tags. This function returns the
     *  number of bins our surface is divided into. We only have one bin for
     *  now, but we could potentially have many, each which different shader
     *  tree masking.
     *
     *  FETODO: Add code here that returns the correct number of bins. If you
     *  only want one material tag for texturing, you can just return 1.
     */
    
    count[0] = 1;
    
    return LXe_OK;
}

LxResult Surface::surf_BinByIndex (unsigned int index, void **ppvObj)
{
    /*
     *  This function is called to get a particular surface bin by index. As
     *  we only have one bin, we always allocate the same object.
     *
     *  FETODO: If you have more than one bin, you'll need to add the correct
     *  code for allocating different bins.
     */
    
    CLxSpawner<SurfElement>  spawner (SERVER_NAME".elmt");
    SurfElement     *element = NULL;
    SurfDef         *definition = NULL;
    
    if (index == 0)
    {
        element = spawner.Alloc (ppvObj);
    
        if (element)
        {
            definition = element->Definition ();
            
            if (definition)
                definition->Copy (&_surf_def);
            
            return LXe_OK;
        }
    }
    
    return LXe_FAILED;
}

LxResult Surface::surf_TagCount (LXtID4 type, unsigned int *count)
{
    /*
     *  This function is called to get the list of polygon tags for all
     *  polygons on the surface. As we only have one bin and one material
     *  tag, we return 1 for material and part, and 0 for everything else.
     *  
     *  FETODO: If you add more than one bin, this function will potentially
     *  need changing.
     */
    
    if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
        count[0] = 1;
    else
        count[0] = 0;
    
    return LXe_OK;
}

LxResult Surface::surf_TagByIndex (LXtID4 type, unsigned int index, const char **stag)
{
    /*
     *  This function is called to get the list of polygon tags for all
     *  polygons on the surface. As we only have one bin with one material
     *  tag and one part tag, we return the default tag.
     *  
     *  FETODO: If you add more than one bin or more than one material tag,
     *  this function will potentially need changing.
     */
    
    if ((type == LXi_PTAG_MATR || type == LXi_PTAG_PART) && index == 0)
    {
        stag[0] = "Default";

        return LXe_OK;
    }
    
    return LXe_OUTOFBOUNDS;
}

LxResult Surface::surf_GLCount (unsigned int *count)
{
    /*
     *  This function is called to return the GL count for the surface we're
     *  generating. The GL count should be the number of triangles generated
     *  by our surface. As our sample surface is just a plane, we can return
     *  a hardcoded value of 2.
     *
     *  FETODO: You will need to expand this to query the returned surface
     *  from the Fabric Engine evaluation.
     */
    
    count[0] = 2;
    
    return LXe_OK;
}
    
SurfDef *Surface::Definition ()
{
    /*
     *  Return a pointer to our surface definition.
     */

    return &_surf_def;
}

/*
 *  The instanceable object is spawned by our modifier. It has one task, which is
 *  to return a surface that matches the current state of the input channels.
 */

class SurfInst : public CLxImpl_Instanceable
{
    public:
        static void initialize ()
        {
            CLxGenericPolymorph *srv = NULL;

            srv = new CLxPolymorph                          <SurfInst>;
            srv->AddInterface       (new CLxIfc_Instanceable        <SurfInst>);

            lx::AddSpawner          (SERVER_NAME".instObj", srv);
        }
    
        LxResult     instable_GetSurface    (void **ppvObj)             LXx_OVERRIDE;
        int      instable_Compare   (ILxUnknownID other)            LXx_OVERRIDE;
    
        SurfDef     *Definition     ();
    
    private:
        SurfDef          _surf_def;
};

LxResult SurfInst::instable_GetSurface (void **ppvObj)
{
    /*
     *  This function is used to allocate the surface. We also copy the cached
     *  channels from the surface definition to the Surface.
     */

    CLxSpawner<Surface>  spawner (SERVER_NAME".surf");
    Surface         *surface = NULL;
    SurfDef         *definition = NULL;

    surface = spawner.Alloc (ppvObj);
    
    if (surface)
    {
        definition = surface->Definition ();
    
        if (definition)
            definition->Copy (&_surf_def);
        
        return LXe_OK;
    }
    
    return LXe_FAILED;
}

int SurfInst::instable_Compare (ILxUnknownID other_obj)
{
    /*
     *  The compare function is used to compare two instanceable objects. It's
     *  identical to strcmp and should either return 0 for identical, or -1/1
     *  to indicate relative position.
     */

    CLxSpawner<SurfInst>     spawner (SERVER_NAME".instObj");
    SurfInst        *other = NULL;

    other = spawner.Cast (other_obj);
    
    if (other)
        return _surf_def.Compare (&other->_surf_def);
    
    return 0;
}

SurfDef *SurfInst::Definition ()
{
    /*
     *  Return a pointer to our surface definition.
     */

    return &_surf_def;
}

/*
 *  Implement the Package Instance.
 */

class Instance : public CLxImpl_PackageInstance, public CLxImpl_SurfaceItem, public CLxImpl_ViewItem3D
{
    public:
        static void initialize ()
        {
            CLxGenericPolymorph *srv = NULL;

            srv = new CLxPolymorph                          <Instance>;
            srv->AddInterface       (new CLxIfc_PackageInstance     <Instance>);
            srv->AddInterface       (new CLxIfc_SurfaceItem         <Instance>);
            srv->AddInterface       (new CLxIfc_ViewItem3D          <Instance>);
            
            lx::AddSpawner          (SERVER_NAME".inst", srv);
        }

        LxResult     pins_Initialize    (ILxUnknownID item_obj, ILxUnknownID super_obj)         LXx_OVERRIDE;
    
        LxResult     isurf_GetSurface   (ILxUnknownID chanRead_obj, unsigned morph, void **ppvObj)  LXx_OVERRIDE;
        LxResult     isurf_Prepare      (ILxUnknownID eval_obj, unsigned *index)            LXx_OVERRIDE;
        LxResult     isurf_Evaluate     (ILxUnknownID attr_obj, unsigned index, void **ppvObj)      LXx_OVERRIDE;

    private:
        CLxUser_Item         _item;
};

LxResult Instance::pins_Initialize (ILxUnknownID item_obj, ILxUnknownID super_obj)
{
    /*
     *  Cache the item for this instance - this is so we can use it for
     *  channel reads and evaluations in the SurfaceItem interface.
     */

    return _item.set (item_obj) ? LXe_OK : LXe_FAILED;
}

LxResult Instance::isurf_GetSurface (ILxUnknownID chanRead_obj, unsigned morph, void **ppvObj)
{
    /*
     *  This function is used to allocate a surface for displaying in the GL
     *  viewport. We're given a channel read object and we're expected to
     *  read the channels needed to generate the surface and then return the
     *  spawned surface. We simply read the instanceable channel and call
     *  its GetSurface function.
     */

    CLxUser_ChannelRead  chan_read (chanRead_obj);
    CLxUser_ValueReference   val_ref;
    CLxLoc_Instanceable  instanceable;

    if (chan_read.Object (_item, CHAN_INSTOBJ, val_ref))
    {
        if (val_ref.Get (instanceable) && instanceable.test ())
            return instanceable.GetSurface (ppvObj);
    }
    
    return LXe_FAILED;
}

LxResult Instance::isurf_Prepare (ILxUnknownID eval_obj, unsigned *index)
{
    /*
     *  This function is used to allocate the channels needed to evaluate a
     *  surface. We just add a single channel; the instanceable. This can be
     *  used to generate the surface.
     */
    
    CLxUser_Evaluation   eval (eval_obj);
    
    index[0] = eval.AddChan (_item, CHAN_INSTOBJ, LXfECHAN_READ);
    
    return LXe_OK;
}

LxResult Instance::isurf_Evaluate (ILxUnknownID attr_obj, unsigned index, void **ppvObj)
{
    /*
     *  This function is used to generate a surface in an evaluated context.
     *  We have a single input channel to the modifier which is the instanceable,
     *  so we read the object and call the GetSurface function.
     */
    
    CLxUser_Attributes   attr (attr_obj);
    CLxUser_ValueReference   val_ref;
    CLxLoc_Instanceable  instanceable;

    if (attr.ObjectRO (index, val_ref))
    {

      feLogDebug("matt - Instance::isurf_Evaluate()");

      if (val_ref.Get (instanceable) && instanceable.test ())
            return instanceable.GetSurface (ppvObj);
        
    }
    
    return LXe_FAILED;
}

/*
 *  Implement the Package.
 */

class Package : public CLxImpl_Package, public CLxImpl_SceneItemListener
{
    public:
        static void initialize ()
        {
            CLxGenericPolymorph *srv = NULL;

            srv = new CLxPolymorph                          <Package>;
            srv->AddInterface       (new CLxIfc_Package         <Package>);
            srv->AddInterface       (new CLxIfc_StaticDesc          <Package>);
            srv->AddInterface       (new CLxIfc_SceneItemListener       <Package>);

            lx::AddServer           (SERVER_NAME, srv);
        }

        Package () : _inst_spawn (SERVER_NAME".inst") {}
    
        LxResult     pkg_SetupChannels  (ILxUnknownID addChan_obj)      LXx_OVERRIDE;
        LxResult     pkg_Attach     (void **ppvObj)             LXx_OVERRIDE;
        LxResult     pkg_TestInterface  (const LXtGUID *guid)           LXx_OVERRIDE;
    
        void         sil_ItemAddChannel (ILxUnknownID item_obj)         LXx_OVERRIDE;

        static LXtTagInfoDesc    descInfo[];
    
    private:
        CLxSpawner<Instance>     _inst_spawn;
};

LxResult Package::pkg_SetupChannels (ILxUnknownID addChan_obj)
{
    /*
     *  Add two channels to our item. One is an objref channel and is used for
     *  caching the instanceable version of our surface, the other is a
     *  dimensions channel that controls the size of the surface item.
     */

    CLxUser_AddChannel   add_chan (addChan_obj);
    
    if (add_chan.test ())
    {
        add_chan.NewChannel (CHAN_INSTOBJ, LXsTYPE_OBJREF);
        
        add_chan.NewChannel (CHAN_DIMENSIONS, LXsTYPE_DISTANCE);
        add_chan.SetDefault (0.5, 0);
    }

    return LXe_OK;
}

LxResult Package::pkg_Attach (void **ppvObj)
{
    /*
     *  Allocate an instance of the package instance.
     */

    _inst_spawn.Alloc (ppvObj);
    
    return ppvObj[0] ? LXe_OK : LXe_FAILED;
}

LxResult Package::pkg_TestInterface (const LXtGUID *guid)
{
    /*
     *  This is called for the various interfaces this package could
     *  potentially support, it should return a result code to indicate if it
     *  implements the specified interface.
     */

    return _inst_spawn.TestInterfaceRC (guid);
}

void Package::sil_ItemAddChannel (ILxUnknownID item_obj)
{
    /*
     *  When user channels are added to our item type, this function will be
     *  called. We use it to invalidate our modifier so that it's reallocated.
     *  We don't need to worry about channels being removed, as the evaluation
     *  system will automatically invalidate the modifier when channels it
     *  is accessing are removed.
     *
     *  NOTE: This won't invalidate other modifiers that have called Prepare
     *  on our SurfaceItem directly.
     */
    
    CLxUser_Item         item (item_obj);
    CLxUser_Scene        scene;
    
    if (item.test () && item.IsA (gItemTypeFix.Type ()))
    {
        if (item.GetContext (scene))
            scene.EvalModInvalidate (SERVER_NAME".mod");
    }
}

LXtTagInfoDesc Package::descInfo[] =
{
    { LXsPKG_SUPERTYPE,     LXsITYPE_LOCATOR },
    { LXsPKG_IS_MASK,       "."      },
    { LXsPKG_INSTANCEABLE_CHANNEL,  CHAN_INSTOBJ     },
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

/*
 *  Implement the Modifier Element and Server. This reads the input channels as
 *  read only channels and output channels as write only channels. It's purpose
 *  is to evaluate the surface definition input channels and output an
 *  instanceable COM object that represents the current state of the surface.
 */

class Element : public CLxItemModifierElement
{
    public:
        Element                 (CLxUser_Evaluation &eval, ILxUnknownID item_obj);
        bool         Test           (ILxUnknownID item_obj)                 LXx_OVERRIDE;
        void         Eval           (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)    LXx_OVERRIDE;
    
    private:
        int          _chan_index;
        SurfDef          _surf_def;
        std::vector <ChannelDef> _user_channels;
};

Element::Element (CLxUser_Evaluation &eval, ILxUnknownID item_obj)
{
    /*
     *  In the constructor, we want to add the input and output channels
     *  required for this modifier. The output is hardcoded as the instanceable
     *  object channel, but for the inputs, we fall through to the Surface
     *  Definition and let that define any channels it needs.
     */

    CLxUser_Item         item (item_obj);
    unsigned         temp = 0;

    if (!item.test())
        return;

    /*
     *  The first channel we want to add is the instanceable object channel
     *  as an output.
     */

    _chan_index = eval.AddChan (item, CHAN_INSTOBJ, LXfECHAN_WRITE);
    
    /*
     *  Call the prepare function on the surface definition to add the
     *  channels it needs.
     */
    
    _surf_def.Prepare (eval, item, &temp);

    /*
     *  Next, we want to grab all of the user channels on the item and cache
     *  them. This is mostly so we can compare the list when the modifier
     *  changes. This could potentially be moved to the Surface Defintion.
     */
    
    userChannels_collect (item, _user_channels);
}

bool Element::Test (ILxUnknownID item_obj)
{
    /*
     *  When the list of user channels for a particular item changes, the
     *  modifier will be invalidated. This function will be called to check
     *  if the modifier we allocated previously matches what we'd allocate
     *  if the Alloc function was called now. We return true if it does. In
     *  our case, we check if the current list of user channels for the
     *  specified item matches what we cached when we allocated the modifier.
     */
    
    CLxUser_Item         item (item_obj);
    std::vector <ChannelDef> user_channels;
    
    if (item.test ())
    {
        userChannels_collect (item, user_channels);
        
        return user_channels.size () == _user_channels.size ();
    }
    
    return false;
}

void Element::Eval (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
{
    /*
     *  The Eval function for the modifier reads input channels and writes
     *  output channels. We allocate an instanceable object and copy the
     *  surface definition to it - then we evaluate it's channels.
     */
    
    CLxSpawner <SurfInst>    spawner (SERVER_NAME".instObj");
    CLxUser_ValueReference   val_ref;
    SurfInst        *instObj = NULL;
    SurfDef         *definition = NULL;
    ILxUnknownID         object = NULL;
    unsigned         temp_chan_index = _chan_index;
    
    if (!eval || !attr)
        return;
    
    /*
     *  Spawn the instanceable object to store in the output channel. We
     *  get the output channel as a writeable Value Reference and then set
     *  the object it contains to our spawned instanceable object.
     */
    
    instObj = spawner.Alloc (object);
    
    if (instObj && attr.ObjectRW (temp_chan_index++, val_ref))
    {
        val_ref.SetObject (object);
    
        /*
         *  Copy the cached surface definition to the surface definition
         *  on the instanceable object.
         */
        
        definition = instObj->Definition ();
        
        if (definition)
        {
            definition->Copy (&_surf_def);
            
            /*
             *  Call Evaluate on the Surface Defintion to get the
             *  channel values required to create the surface.
             */
            
            definition->Evaluate (attr, temp_chan_index);
        }
    }
}

class Modifier : public CLxItemModifierServer
{
    public:
        static void initialize()
        {
            CLxExport_ItemModifierServer <Modifier> (SERVER_NAME".mod");
        }
    
        const char  *ItemType       ()                          LXx_OVERRIDE;
    
        CLxItemModifierElement *Alloc       (CLxUser_Evaluation &eval, ILxUnknownID item_obj)   LXx_OVERRIDE;
};

const char * Modifier::ItemType ()
{
    /*
     *  The modifier should only associate itself with this item type.
     */

    return SERVER_NAME;
}

CLxItemModifierElement * Modifier::Alloc (CLxUser_Evaluation &eval, ILxUnknownID item)
{
    /*
     *  Allocate and return the modifier element.
     */

    return new Element (eval, item);
}

void initialize ()
{
    Instance::initialize ();
    Package::initialize ();
    Modifier::initialize ();
    SurfInst::initialize ();
    Surface::initialize ();
    SurfElement::initialize ();
}

};  // End Namespace.

//void initialize ()
//{
//    Surface_Sample_Fix::initialize ();
//}
