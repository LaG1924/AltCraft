#pragma once

#include <cstddef>
#include <vector>
#include <iostream>
#include <zlib.h>
#include <fstream>
#include "../utility/utility.h"

namespace nbt {
    enum TagType {
        End,        //nullptr
        Byte,       //int8_t
        Short,      //int16_t
        Int,        //int32_t
        Long,       //int64_t
        Float,      //float
        Double,     //double
        ByteArray,  //std::vector<signed char>
        String,     //std::string
        List,       //std::vector<NbtTag>
        Compound,   //std::vector<NbtTag>
        IntArray,   //std::vector<int32_t>
        Unknown,    //dummy value
    };

    class NbtTag;

    typedef std::vector<NbtTag> compound_t;

    typedef std::string string_t;

    typedef std::vector<signed char> byteArray_t;

    typedef std::vector<int> intArray_t;

    class NbtTag {
        TagType type = Unknown;
        string_t name = "";
        unsigned char *data = nullptr;
    public:
        NbtTag(TagType type, string_t name) : type(type), name(name) {
            switch (type) {
                case End:
                    data = nullptr;
                    break;
                case Compound:
                    data = (unsigned char *) new compound_t;
                    break;
                case String:
                    data = (unsigned char *) new string_t;
                    break;
                case Int:
                    data = (unsigned char *) new int32_t;
                    break;
                case Long:
                    data = (unsigned char *) new int64_t;
                    break;
                case Byte:
                    data = (unsigned char *) new int8_t;
                    break;
                case Short:
                    data = (unsigned char *) new int16_t;
                    break;
                case Float:
                    data = (unsigned char *) new float;
                    break;
                case Double:
                    data = (unsigned char *) new double;
                    break;
                case ByteArray:
                    data = (unsigned char *) new byteArray_t;
                    break;
                case List:
                    data = (unsigned char *) new compound_t;
                    break;
                case IntArray:
                    data = (unsigned char *) new intArray_t;
            }
        }

        NbtTag(const NbtTag &other) : type(other.type), name(other.name) {
            switch (type) {
                case Byte:
                    data = (unsigned char *) new int8_t;
                    *((int8_t *) data) = *((int8_t *) other.data);
                    break;
                case Short:
                    data = (unsigned char *) new int16_t;
                    *((int16_t *) data) = *((int16_t *) other.data);
                    break;
                case Int:
                    data = (unsigned char *) new int32_t;
                    *((int32_t *) data) = *((int32_t *) other.data);
                    break;
                case Long:
                    data = (unsigned char *) new int64_t;
                    *((int64_t *) data) = *((int64_t *) other.data);
                    break;
                case Float:
                    data = (unsigned char *) new float;
                    *((float *) data) = *((float *) other.data);
                    break;
                case Double:
                    data = (unsigned char *) new double;
                    *((double *) data) = *((double *) other.data);
                    break;
                case ByteArray:
                    data = (unsigned char *) new byteArray_t;
                    *((std::vector<signed char> *) data) = *((std::vector<signed char> *) other.data);
                    break;
                case String:
                    data = (unsigned char *) new string_t;
                    *((std::string *) data) = *((std::string *) other.data);
                    break;
                case List:
                    data = (unsigned char *) new compound_t;
                    *((std::vector<NbtTag> *) data) = *((std::vector<NbtTag> *) other.data);
                    break;
                case Compound:
                    data = (unsigned char *) new compound_t;
                    *((std::vector<NbtTag> *) data) = *((std::vector<NbtTag> *) other.data);
                    break;
                case IntArray:
                    data = (unsigned char *) new intArray_t;
                    *((std::vector<int> *) data) = *((std::vector<int> *) other.data);
                    break;
            }
        }

        ~NbtTag() {
            switch (type) {
                case Byte:
                    delete ((int8_t *) data);
                    break;
                case Short:
                    delete ((int16_t *) data);
                    break;
                case Int:
                    delete ((int32_t *) data);
                    break;
                case Long:
                    delete ((int64_t *) data);
                    break;
                case Float:
                    delete ((float *) data);
                    break;
                case Double:
                    delete ((double *) data);
                    break;
                case ByteArray:
                    delete ((std::vector<signed char> *) data);
                    break;
                case String:
                    delete ((std::string *) data);
                    break;
                case List:
                    delete ((std::vector<NbtTag> *) data);
                    break;
                case Compound:
                    delete ((std::vector<NbtTag> *) data);
                    break;
                case IntArray:
                    delete ((std::vector<int> *) data);
                    break;
            }
        };

        void swap(NbtTag &other) {
            std::swap(other.data, data);
            std::swap(other.name, name);
            std::swap(other.type, type);
        }

        NbtTag &operator=(NbtTag other) {
            other.swap(*this);
            return *this;
        }

        TagType GetType() const{
            return type;
        }

        string_t GetName() const{
            return name;
        }


        string_t &GetString() {
            string_t &val = *reinterpret_cast<std::string *>(data);
            return val;
        }

        compound_t &GetCompound() {
            std::vector<NbtTag> &val = *reinterpret_cast<std::vector<NbtTag> *>(data);
            return val;
        }

        compound_t &GetList() {
            std::vector<NbtTag> &val = *reinterpret_cast<std::vector<NbtTag> *>(data);
            return val;
        }

        int64_t &GetLong() {
            int64_t &val = *reinterpret_cast<int64_t *>(data);
            return val;
        }

        float &GetFloat() {
            float &val = *reinterpret_cast<float *>(data);
            return val;
        }

        double &GetDouble() {
            double &val = *reinterpret_cast<double *>(data);
            return val;
        }

        byteArray_t &GetByteArray() {
            auto &val = *reinterpret_cast<byteArray_t *>(data);
            return val;
        }

        intArray_t &GetIntArray() {
            auto &val = *reinterpret_cast<intArray_t *>(data);
            return val;
        }

        int16_t &GetShort() {
            auto &val = *reinterpret_cast<int16_t *>(data);
            return val;
        }

        int32_t &GetInt() {
            auto &val = *reinterpret_cast<int32_t *>(data);
            return val;
        }

        int8_t &GetByte() {
            auto &val = *reinterpret_cast<int8_t *>(data);
            return val;
        }
    };

    NbtTag ParseTag(unsigned char *data, size_t &size, TagType listItemType = Unknown) {
        size = 0;
        TagType type;
        if (listItemType == Unknown) {
            type = (TagType) *data;
            data += 1;
            size += 1;
        } else
            type = listItemType;
        string_t name;
        if (listItemType == Unknown && type != End) {
            short nameLen = *((short *) data);
            data += 2;
            size += 2;
            endswap(&nameLen);
            name = std::string((char *) data, nameLen);
            data += nameLen;
            size += nameLen;
        }
        NbtTag tag(type, name);
        switch (type) {
            case Compound: {
                do {
                    size_t s;
                    tag.GetCompound().push_back(ParseTag(data, s));
                    data += s;
                    size += s;
                } while (tag.GetCompound().back().GetType() != End);
                tag.GetCompound().pop_back();
                return tag;
            }
            case String: {
                short len = *((short *) data);
                data += 2;
                size += 2;
                endswap(&len);
                string_t str((char *) data, len);
                data += len;
                size += len;
                tag.GetString() = str;
                return tag;
            }
            case End:
                return tag;
            case Long:
                tag.GetLong() = *((int64_t *) data);
                endswap(&tag.GetLong());
                data += 8;
                size += 8;
                return tag;
            case Short:
                tag.GetShort() = *((int16_t *) data);
                endswap(&tag.GetShort());
                data += 2;
                size += 2;
                return tag;
            case Float:
                tag.GetFloat() = *((float *) data);
                endswap(&tag.GetFloat());
                data += 4;
                size += 4;
                return tag;
            case Double:
                tag.GetDouble() = *((double *) data);
                endswap(&tag.GetDouble());
                data += 8;
                size += 8;
                return tag;
            case Byte:
                tag.GetByte() = *((int8_t *) data);
                endswap(&tag.GetByte());
                data += 1;
                size += 1;
                return tag;
            case Int:
                tag.GetInt() = *((int32_t *) data);
                endswap(&tag.GetInt());
                data += 4;
                size += 4;
                return tag;
            case List: {
                TagType listType = *((TagType *) data);
                data += 1;
                size += 1;
                int32_t listLength = *((int32_t *) data);
                endswap(&listLength);
                data += 4;
                size += 4;
                for (int i = 0; i < listLength; i++) {
                    size_t s = 0;
                    std::vector<NbtTag> &vec = tag.GetCompound();
                    vec.push_back(ParseTag(data, s, listType));
                    data += s;
                    size += s;
                }
                return tag;
            }
            case ByteArray: {
                int32_t arrLength = *((int32_t *) data);
                endswap(&arrLength);
                data += 4;
                size += 4;
                for (int i = 0; i < arrLength; i++) {
                    signed char val = (signed char) data[i];
                    std::vector<signed char> &vec = tag.GetByteArray();
                    vec.push_back(val);
                }
                data += arrLength;
                size += arrLength;
                return tag;
            }
            default:
                throw 13;
        }
    }

    NbtTag ParseTag(unsigned char *data, size_t *optionalSize = nullptr) {
        size_t s = 0;
        size_t &size = (optionalSize ? *optionalSize : s);
        return ParseTag(data, size);
    }

    std::vector<unsigned char> Decompress(unsigned char *data, size_t dataLen) {
        const size_t decompBuffSize = 1024 * 16;
        unsigned char *decompBuff = new unsigned char[decompBuffSize];
        std::vector<unsigned char> uncompressed;
        for (int i = 0; i < decompBuffSize; i++)
            decompBuff[i] = 0;


        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.next_in = data;
        stream.avail_in = dataLen;
        stream.next_out = decompBuff;
        stream.avail_out = decompBuffSize;

        if (inflateInit2(&stream, 15 + 32) != Z_OK) {
            delete[] decompBuff;
            throw 171;
        }

        int res;
        do {
            stream.avail_out = decompBuffSize;

            switch ((res = inflate(&stream, Z_NO_FLUSH))) {
                case Z_MEM_ERROR:
                    throw 172;
                case Z_DATA_ERROR:
                    throw 173;
                case Z_NEED_DICT:
                    throw 174;
            }

            uncompressed.resize(uncompressed.size() + decompBuffSize);
            std::copy(decompBuff, decompBuff + decompBuffSize, uncompressed.end() - decompBuffSize);
        } while (stream.avail_out == 0);
        if (res != Z_STREAM_END)
            throw 175;
        if (inflateEnd(&stream) != Z_OK)
            throw 176;
        delete[] decompBuff;
        return uncompressed;
    }

    NbtTag ParseCompressed(unsigned char *data, size_t dataLen) {
        auto uncompressed = Decompress(data, dataLen);
        NbtTag root = ParseTag(uncompressed.data());
        return root;
    }

    NbtTag Parse(unsigned char *data, size_t dataLen) {
        bool isCompressed = *data!=10;
        if (isCompressed)
            return ParseCompressed(data,dataLen);
        else
            return ParseTag(data);
    }

    void PrintTree(NbtTag &tree, int deepness = 0, std::ostream &ostream = std::cout) {
        ostream << std::string(deepness, '\t') << "Tag ";
        switch (tree.GetType()) {
            case Byte:
                ostream << "byte";
                break;
            case Short:
                ostream << "short";
                break;
            case Int:
                ostream << "int";
                break;
            case Long:
                ostream << "long";
                break;
            case Float:
                ostream << "float";
                break;
            case Double:
                ostream << "double";
                break;
            case ByteArray:
                ostream << "byte array";
                break;
            case String:
                ostream << "string";
                break;
            case List:
                ostream << "list";
                break;
            case Compound:
                ostream << "compound";
                break;
            case IntArray:
                ostream << "int array";
                break;
            case End:
                ostream << "end";
                break;
        }
        if (tree.GetName().length() > 0)
            ostream << " (" << tree.GetName() << ")";
        ostream << ": ";

        if (tree.GetType() == Compound || tree.GetType() == List) {
            std::vector<NbtTag> &vec = (tree.GetType() == Compound ? tree.GetCompound() : tree.GetList());
            ostream << vec.size() << " entries {" << std::endl;
            for (auto it = vec.begin(); it != vec.end(); ++it) {
                PrintTree(*it, deepness + 1, std::cout);
            }
            ostream << std::string(deepness, '\t') << "}" << std::endl;
            return;
        } else {
            switch (tree.GetType()) {
                case Int:
                    ostream << tree.GetInt();
                    break;
                case String:
                    ostream << "\"" << tree.GetString() << "\"";
                    break;
                case Double:
                    ostream << tree.GetDouble();
                    break;
                case Float:
                    ostream << tree.GetFloat();
                    break;
                case Short:
                    ostream << tree.GetShort();
                    break;
                case Byte:
                    ostream << (int) tree.GetByte();
                    break;
                case Long:
                    ostream << tree.GetLong();
                    break;
                case ByteArray:
                    ostream << "[" << tree.GetByteArray().size() << " bytes]: ";
                    for (int i = 0; i < (tree.GetByteArray().size() > 10 ? 10 : tree.GetByteArray().size()); i++) {
                        ostream << std::hex << "0x" << (tree.GetByteArray()[i] > 15 ? "" : "0")
                                << (int) tree.GetByteArray()[i]
                                << std::dec << " ";
                    }
                    break;
                case IntArray:
                    break;
            }
            ostream << std::endl;
        }
    }
}