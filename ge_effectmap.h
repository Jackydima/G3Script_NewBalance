#ifndef GE_EFFECTMAP_H_INCLUDED
#define GE_EFFECTMAP_H_INCLUDED

#include "SharedBase.h"

class GE_DLLIMPORT gCEffectCommand;

class GE_DLLIMPORT gCEffectCommandSequence {

    public: virtual ~gCEffectCommandSequence ( void );

    public:
        gCEffectCommandSequence ( gCEffectCommandSequence const& );
        gCEffectCommandSequence ( void );

    public:
        gCEffectCommandSequence& operator=( gCEffectCommandSequence const& );

    public:
        GE_PADDING ( 0xC );
        gCEffectCommand* gCEffectCommandSequence::operator[]( int )const;
        void CopyFrom ( class gCEffectCommandSequence const& );
        GEFloat GetEndTime ( void )const;
        gCEffectCommand* GetItem ( int )const;
        GEInt GetItemCount ( void )const;

        // TODO: InsertItem Imports
    protected: 
        static int __cdecl CompareItems ( void const* , void const* );
        void Destroy ( void );
        
        GEInt InsertItem ( struct gSEffectCommand_CreateDecal const& );
        GEInt InsertItem ( struct gSEffectCommand_EarthQuake const& );
        int __thiscall gCEffectCommandSequence::InsertItem ( struct gSEffectCommand_KillEntity const& );
        int __thiscall gCEffectCommandSequence::InsertItem ( struct gSEffectCommand_ModifyEntity const& );
        int __thiscall gCEffectCommandSequence::InsertItem ( struct gSEffectCommand_MusicTrigger const& );
        int __thiscall gCEffectCommandSequence::InsertItem ( struct gSEffectCommand_PlaySound const& );
        int __thiscall gCEffectCommandSequence::InsertItem ( struct gSEffectCommand_PlayVoice const& );

};

class GE_DLLIMPORT gCEffectMap 
{
    public: virtual ~gCEffectMap(void);

    public:
        gCEffectMap(gCEffectMap const &);
        gCEffectMap(void);

    public:
        gCEffectMap & operator=(gCEffectMap const &);

    public:
        GE_PADDING ( 0xC );
        GEInt Entries;
        bTValMap<bCString,gCEffectCommandSequence>::bCIterator Begin(void);
        bTValMap<bCString,gCEffectCommandSequence>::bCConstIterator Begin(void)const;
        bool Contains(bCString const &)const;
        bTValMap<bCString,gCEffectCommandSequence>::bCIterator End(void);
        bTValMap<bCString,gCEffectCommandSequence>::bCConstIterator End(void)const;
        gCEffectCommandSequence * InsertNewAt(bCString const &);
        bool Load(bCString const &);
        gCEffectCommandSequence * Lookup(bCString const &);
        bool Lookup(bCString const &,gCEffectCommandSequence &)const;
        void Read(bCIStream &);
        void RemoveAll(void);
        bool RemoveAt(bCString const &);
        bool Save(bCString const &)const;
        void Write(bCOStream &)const;

    protected:
        void Destroy(void);
        void Invalidate(void);

};

#endif
