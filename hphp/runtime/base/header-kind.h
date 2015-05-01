/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_HEADER_KIND_H_
#define incl_HPHP_HEADER_KIND_H_

namespace HPHP {

enum class HeaderKind : uint8_t {
  // ArrayKind aliases
  Packed, Struct, Mixed, Empty, Apc, Globals, Proxy,
  // Other ordinary refcounted heap objects
  String, Resource, Ref,
  Object, ResumableObj, AwaitAllWH,
  Vector, Map, Set, Pair, ImmVector, ImmMap, ImmSet,
  ResumableFrame, // ResumableNode followed by Frame, Resumable, ObjectData
  NativeData, // a NativeData header preceding an HNI ObjectData
  SmallMalloc, // small smart_malloc'd block
  BigMalloc, // big smart_malloc'd block
  BigObj, // big size-tracked object (valid header follows BigNode)
  Free, // small block in a FreeList
  Hole, // wasted space not in any freelist
  Debug // a DebugHeader
};
const unsigned NumHeaderKinds = unsigned(HeaderKind::Debug) + 1;

/*
 * Common header for all heap-allocated objects. Layout is carefully
 * designed to allow overlapping with the second word of a TypedValue,
 * or to follow a C++ defined vptr.
 */
template<class T = uint16_t> struct HeaderWord {
  union {
    struct {
      T aux;
      HeaderKind kind;
      uint8_t marks;
    };
    uint32_t w;
  };
  static uint32_t pack(HeaderKind kind) {
    return static_cast<uint32_t>(kind) << (8 * offsetof(HeaderWord, kind));
  }
  static uint32_t pack(T aux, HeaderKind kind) {
    return pack(kind) | static_cast<uint16_t>(aux);
    static_assert(sizeof(aux) == 2, "");
  }
  void init(T aux, HeaderKind kind) { w = pack(aux, kind); }
  void init(HeaderKind kind) { w = pack(kind); }
};

constexpr auto HeaderOffset = sizeof(void*);
constexpr auto HeaderKindOffset = HeaderOffset + offsetof(HeaderWord<>, kind);

inline bool isObjectKind(HeaderKind k) {
  return uint8_t(k) >= uint8_t(HeaderKind::Object) &&
         uint8_t(k) <= uint8_t(HeaderKind::ImmSet);
}

enum class CollectionType : uint8_t { // Subset of possible HeaderKind values
  // Values must be contiguous integers (for ArrayIter::initFuncTable).
  Vector = uint8_t(HeaderKind::Vector),
  Map = uint8_t(HeaderKind::Map),
  Set = uint8_t(HeaderKind::Set),
  Pair = uint8_t(HeaderKind::Pair),
  ImmVector = uint8_t(HeaderKind::ImmVector),
  ImmMap = uint8_t(HeaderKind::ImmMap),
  ImmSet = uint8_t(HeaderKind::ImmSet),
};

inline bool isVectorCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector || ctype == CollectionType::ImmVector;
}
inline bool isMapCollection(CollectionType ctype) {
  return ctype == CollectionType::Map || ctype == CollectionType::ImmMap;
}
inline bool isSetCollection(CollectionType ctype) {
  return ctype == CollectionType::Set || ctype == CollectionType::ImmSet;
}
inline bool isValidCollection(CollectionType ctype) {
  return uint8_t(ctype) >= uint8_t(CollectionType::Vector) &&
         uint8_t(ctype) <= uint8_t(CollectionType::ImmSet);
}
inline bool isMutableCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector ||
         ctype == CollectionType::Map ||
         ctype == CollectionType::Set;
}
inline bool isImmutableCollection(CollectionType ctype) {
  return !isMutableCollection(ctype);
}

inline bool collectionAllowsIntStringKeys(CollectionType ctype) {
  return isSetCollection(ctype) || isMapCollection(ctype);
}

}

#endif
