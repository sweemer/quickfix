/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) 2001-2014
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef FIX_FIELD
#define FIX_FIELD

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "FieldConvertors.h"
#include "FieldNumbers.h"
#include "FieldTypes.h"
#include "Utility.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <string_view>

namespace FIX {
/**
 * Base representation of all Field classes.
 *
 * This base class is the lowest common denominator of all fields.  It
 * keeps all fields in its most generic string representation with its
 * integer tag.
 */
class FieldBase {

  /// Class used to store field metrics like total length and checksum
  class field_metrics {
  public:
    field_metrics(const int length, const int checksum)
        : m_length(length),
          m_checksum(checksum) {}

    int getLength() const { return m_length; }

    int getCheckSum() const { return m_checksum; }

    bool isValid() const { return m_length > 0; }

  private:
    int m_length;
    int m_checksum;
  };

  friend class Message;

  /// Constructor which also calculates field metrics
  FieldBase(
      int tag,
      std::string::const_iterator valueStart,
      std::string::const_iterator valueEnd,
      std::string::const_iterator tagStart,
      std::string::const_iterator tagEnd)
      : m_tag(tag),
        m_string(valueStart, valueEnd),
        m_metrics(calculateMetrics(tagStart, tagEnd)) {}

public:
  FieldBase(int tag, const std::string &string)
      : m_tag(tag),
        m_string(string),
        m_metrics(no_metrics()) {}

  virtual ~FieldBase() {}

  FieldBase(const FieldBase &rhs)
      : m_tag(rhs.getTag()),
        m_string(rhs.m_string),
        m_metrics(rhs.m_metrics) {}

  FieldBase &operator=(const FieldBase &rhs) {
    m_tag = rhs.getTag();
    m_string = rhs.m_string;
    m_metrics = rhs.m_metrics;
    m_data.clear();

    return *this;
  }

  void swap(FieldBase &rhs) {
    std::swap(m_tag, rhs.m_tag);
    std::swap(m_metrics, rhs.m_metrics);
    m_string.swap(rhs.m_string);
    m_data.swap(rhs.m_data);
  }

  void setTag(int tag) {
    m_tag = tag;
    m_metrics = no_metrics();
    m_data.clear();
  }

  [[deprecated("Use setTag")]]
  void setField(int field) {
    setTag(field);
  }

  void setString(const std::string &string) {
    m_string = string;
    m_metrics = no_metrics();
    m_data.clear();
  }

  /// Get the fields integer tag.
  int getTag() const { return m_tag; }

  [[deprecated("Use getTag")]]
  int getField() const {
    return getTag();
  }

  /// Get the string representation of the fields value.
  const std::string &getString() const { return m_string; }

  /// Get the string representation of the Field (i.e.) 55=MSFT[SOH]
  const std::string &getFixString() const {
    if (m_data.empty()) {
      encodeTo(m_data);
    }

    return m_data;
  }

  /// Get the length of the fields string representation
  int getLength() const {
    calculate();
    return m_metrics.getLength();
  }

  /// Get the total value the fields characters added together
  int getTotal() const {
    calculate();
    return m_metrics.getCheckSum();
  }

  /// Compares fields based on their tag numbers
  bool operator<(const FieldBase &field) const { return m_tag < field.m_tag; }

private:
  void calculate() const {
    if (m_metrics.isValid()) {
      return;
    }

    m_metrics = calculateMetrics(getFixString());
  }

  /// Serializes string representation of the Field to input string
  void encodeTo(std::string &result) const {
    size_t tagLength = FIX::number_of_symbols_in(m_tag);
    size_t totalLength = tagLength + m_string.length() + 2;

    result.resize(totalLength);

    char *buf = (char *)result.c_str();
    FIX::integer_to_string(buf, tagLength, m_tag);

    buf[tagLength] = '=';
    memcpy(buf + tagLength + 1, m_string.data(), m_string.length());
    buf[totalLength - 1] = '\001';
  }

  static field_metrics no_metrics() { return field_metrics(0, 0); }

  /// Calculate metrics for any input string
  static field_metrics calculateMetrics(
      std::string::const_iterator const start,
      std::string::const_iterator const end) {
    int checksum = 0;
    for (std::string::const_iterator str = start; str != end; ++str) {
      checksum += (unsigned char)(*str);
    }

    return field_metrics(static_cast<int>(std::distance(start, end)), checksum);
  }

  static field_metrics calculateMetrics(const std::string &field) {
    return calculateMetrics(field.begin(), field.end());
  }

  int m_tag;
  std::string m_string;
  mutable std::string m_data;
  mutable field_metrics m_metrics;
};
/*! @} */

inline std::ostream &operator<<(std::ostream &stream, const FieldBase &field) {
  stream << field.getString();
  return stream;
}

inline void swap(FieldBase &lhs, FieldBase &rhs) { lhs.swap(rhs); }

/**
 * MSC doesn't support partial template specialization so we have this.
 * this is here to provide equality checking against native char arrays.
 */
class StringField : public FieldBase {
public:
  explicit StringField(int field, const std::string &data)
      : FieldBase(field, data) {}
  StringField(int field)
      : FieldBase(field, "") {}

  void setValue(const std::string &value) { setString(value); }
  const std::string &getValue() const { return getString(); }
  operator const std::string &() const { return getString(); }
  operator std::string_view() const { return getString(); }

  bool operator<(const StringField &rhs) const { return getString() < rhs.getString(); }
  bool operator>(const StringField &rhs) const { return getString() > rhs.getString(); }
  bool operator==(const StringField &rhs) const { return getString() == rhs.getString(); }
  bool operator!=(const StringField &rhs) const { return getString() != rhs.getString(); }
  bool operator<=(const StringField &rhs) const { return getString() <= rhs.getString(); }
  bool operator>=(const StringField &rhs) const { return getString() >= rhs.getString(); }
  friend bool operator<(const StringField &, const char *);
  friend bool operator<(const char *, const StringField &);
  friend bool operator>(const StringField &, const char *);
  friend bool operator>(const char *, const StringField &);
  friend bool operator==(const StringField &, const char *);
  friend bool operator==(const char *, const StringField &);
  friend bool operator!=(const StringField &, const char *);
  friend bool operator!=(const char *, const StringField &);
  friend bool operator<=(const StringField &, const char *);
  friend bool operator<=(const char *, const StringField &);
  friend bool operator>=(const StringField &, const char *);
  friend bool operator>=(const char *, const StringField &);

  friend bool operator<(const StringField &, const std::string &);
  friend bool operator<(const std::string &, const StringField &);
  friend bool operator>(const StringField &, const std::string &);
  friend bool operator>(const std::string &, const StringField &);
  friend bool operator==(const StringField &, const std::string &);
  friend bool operator==(const std::string &, const StringField &);
  friend bool operator!=(const StringField &, const std::string &);
  friend bool operator!=(const std::string &, const StringField &);
  friend bool operator<=(const StringField &, const std::string &);
  friend bool operator<=(const std::string &, const StringField &);
  friend bool operator>=(const StringField &, const std::string &);
  friend bool operator>=(const std::string &, const StringField &);
};

inline bool operator<(const StringField &lhs, const char *rhs) { return lhs.getValue() < rhs; }
inline bool operator<(const char *lhs, const StringField &rhs) { return lhs < rhs.getValue(); }
inline bool operator>(const StringField &lhs, const char *rhs) { return lhs.getValue() > rhs; }
inline bool operator>(const char *lhs, const StringField &rhs) { return lhs > rhs.getValue(); }
inline bool operator==(const StringField &lhs, const char *rhs) { return lhs.getValue() == rhs; }
inline bool operator==(const char *lhs, const StringField &rhs) { return lhs == rhs.getValue(); }
inline bool operator!=(const StringField &lhs, const char *rhs) { return lhs.getValue() != rhs; }
inline bool operator!=(const char *lhs, const StringField &rhs) { return lhs != rhs.getValue(); }
inline bool operator<=(const StringField &lhs, const char *rhs) { return lhs.getValue() <= rhs; }
inline bool operator<=(const char *lhs, const StringField &rhs) { return lhs <= rhs.getValue(); }
inline bool operator>=(const StringField &lhs, const char *rhs) { return lhs.getValue() >= rhs; }
inline bool operator>=(const char *lhs, const StringField &rhs) { return lhs >= rhs.getValue(); }

inline bool operator<(const StringField &lhs, const std::string &rhs) { return lhs.getValue() < rhs; }
inline bool operator<(const std::string &lhs, const StringField &rhs) { return lhs < rhs.getValue(); }
inline bool operator>(const StringField &lhs, const std::string &rhs) { return lhs.getValue() > rhs; }
inline bool operator>(const std::string &lhs, const StringField &rhs) { return lhs > rhs.getValue(); }
inline bool operator==(const StringField &lhs, const std::string &rhs) { return lhs.getValue() == rhs; }
inline bool operator==(const std::string &lhs, const StringField &rhs) { return lhs == rhs.getValue(); }
inline bool operator!=(const StringField &lhs, const std::string &rhs) { return lhs.getValue() != rhs; }
inline bool operator!=(const std::string &lhs, const StringField &rhs) { return lhs != rhs.getValue(); }
inline bool operator<=(const StringField &lhs, const std::string &rhs) { return lhs.getValue() <= rhs; }
inline bool operator<=(const std::string &lhs, const StringField &rhs) { return lhs <= rhs.getValue(); }
inline bool operator>=(const StringField &lhs, const std::string &rhs) { return lhs.getValue() >= rhs; }
inline bool operator>=(const std::string &lhs, const StringField &rhs) { return lhs >= rhs.getValue(); }

/// Field that contains a character value
class CharField : public FieldBase {
public:
  explicit CharField(int field, char data)
      : FieldBase(field, CharConvertor::convert(data)) {}
  CharField(int field)
      : FieldBase(field, "") {}

  void setValue(char value) { setString(CharConvertor::convert(value)); }
  char getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return CharConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator char() const { return getValue(); }
};

/// Field that contains a double value
class DoubleField : public FieldBase {
public:
  explicit DoubleField(int field, double data, int padding = 0)
      : FieldBase(field, DoubleConvertor::convert(data, padding)) {}
  DoubleField(int field)
      : FieldBase(field, "") {}

  void setValue(double value, int padding = 0) { setString(DoubleConvertor::convert(value, padding)); }
  double getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return DoubleConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator double() const { return getValue(); }
};

/// Field that contains an integer value
class IntField : public FieldBase {
public:
  explicit IntField(int field, int data)
      : FieldBase(field, IntConvertor::convert(data)) {}
  IntField(int field)
      : FieldBase(field, "") {}

  void setValue(int value) { setString(IntConvertor::convert(value)); }
  int getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return IntConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator int() const { return getValue(); }
};

// A generic template-based int field type would be more elegant; a specific field
// specialization could be declared for any required intN_t but this does not go
// well with SWIG (it looks breaking the inheritance chain on a template).
//
/// Field that contains a 64-bit integer value
class Int64Field : public FieldBase {
public:
  explicit Int64Field(int field, int64_t data)
      : FieldBase(field, Int64Convertor::convert(data)) {}
  Int64Field(int field)
      : FieldBase(field, "") {}

  void setValue(int64_t value) { setString(Int64Convertor::convert(value)); }
  int64_t getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return Int64Convertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator int64_t() const { return getValue(); }
};

/// Field that contains a 64-bit integer value
class UInt64Field : public FieldBase {
public:
  explicit UInt64Field(int field, uint64_t data)
      : FieldBase(field, UInt64Convertor::convert(data)) {}
  UInt64Field(int field)
      : FieldBase(field, "") {}

  void setValue(uint64_t value) { setString(UInt64Convertor::convert(value)); }
  uint64_t getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return UInt64Convertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator uint64_t() const { return getValue(); }
};

/// Field that contains a boolean value
class BoolField : public FieldBase {
public:
  explicit BoolField(int field, bool data)
      : FieldBase(field, BoolConvertor::convert(data)) {}
  BoolField(int field)
      : FieldBase(field, "") {}

  void setValue(bool value) { setString(BoolConvertor::convert(value)); }
  bool getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return BoolConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator bool() const { return getValue(); }
};

/// Field that contains a UTC time stamp value
class UtcTimeStampField : public FieldBase {
public:
  explicit UtcTimeStampField(int field, const UtcTimeStamp &data, int precision = 0)
      : FieldBase(field, UtcTimeStampConvertor::convert(data, precision)) {}
  UtcTimeStampField(int field, int precision = 0)
      : FieldBase(field, UtcTimeStampConvertor::convert(UtcTimeStamp::now(), precision)) {}

  void setValue(const UtcTimeStamp &value) { setString(UtcTimeStampConvertor::convert(value)); }
  UtcTimeStamp getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return UtcTimeStampConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator UtcTimeStamp() const { return getValue(); }

  bool operator<(const UtcTimeStampField &rhs) const { return getValue() < rhs.getValue(); }
  bool operator==(const UtcTimeStampField &rhs) const { return getValue() == rhs.getValue(); }
  bool operator!=(const UtcTimeStampField &rhs) const { return getValue() != rhs.getValue(); }
};

/// Field that contains a UTC date value
class UtcDateField : public FieldBase {
public:
  explicit UtcDateField(int field, const UtcDate &data)
      : FieldBase(field, UtcDateConvertor::convert(data)) {}
  UtcDateField(int field)
      : FieldBase(field, UtcDateConvertor::convert(UtcDate())) {}

  void setValue(const UtcDate &value) { setString(UtcDateConvertor::convert(value)); }
  UtcDate getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return UtcDateConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator UtcDate() const { return getValue(); }

  bool operator<(const UtcDateField &rhs) const { return getValue() < rhs.getValue(); }
  bool operator==(const UtcDateField &rhs) const { return getValue() == rhs.getValue(); }
  bool operator!=(const UtcDateField &rhs) const { return getValue() != rhs.getValue(); }
};

/// Field that contains a UTC time value
class UtcTimeOnlyField : public FieldBase {
public:
  explicit UtcTimeOnlyField(int field, const UtcTimeOnly &data, int precision = 0)
      : FieldBase(field, UtcTimeOnlyConvertor::convert(data, precision)) {}
  UtcTimeOnlyField(int field, int precision = 0)
      : FieldBase(field, UtcTimeOnlyConvertor::convert(UtcTimeOnly(), precision)) {}

  void setValue(const UtcTimeOnly &value) { setString(UtcTimeOnlyConvertor::convert(value)); }
  UtcTimeOnly getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return UtcTimeOnlyConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator UtcTimeOnly() const { return getValue(); }

  bool operator<(const UtcTimeOnlyField &rhs) const { return getValue() < rhs.getValue(); }
  bool operator==(const UtcTimeOnlyField &rhs) const { return getValue() == rhs.getValue(); }
  bool operator!=(const UtcTimeOnlyField &rhs) const { return getValue() != rhs.getValue(); }
};

/// Field that contains a checksum value
class CheckSumField : public FieldBase {
public:
  explicit CheckSumField(int field, int data)
      : FieldBase(field, CheckSumConvertor::convert(data)) {}
  CheckSumField(int field)
      : FieldBase(field, "") {}

  void setValue(int value) { setString(CheckSumConvertor::convert(value)); }
  int getValue() const EXCEPT(IncorrectDataFormat) {
    try {
      return CheckSumConvertor::convert(getString());
    } catch (FieldConvertError &) {
      throw IncorrectDataFormat(getTag(), getString());
    }
  }
  operator int() const { return getValue(); }
};

typedef DoubleField PriceField;
typedef DoubleField AmtField;
typedef DoubleField QtyField;
typedef StringField CurrencyField;
typedef StringField MultipleValueStringField;
typedef StringField MultipleStringValueField;
typedef StringField MultipleCharValueField;
typedef StringField ExchangeField;
typedef StringField LocalMktDateField;
typedef StringField DataField;
typedef DoubleField FloatField;
typedef DoubleField PriceOffsetField;
typedef StringField MonthField;
typedef StringField MonthYearField;
typedef StringField DayOfMonthField;
typedef UtcDateField UtcDateOnlyField;
typedef IntField LengthField;
typedef IntField NumInGroupField;
typedef UInt64Field SeqNumField;
typedef IntField TagNumField;
typedef DoubleField PercentageField;
typedef StringField CountryField;
typedef StringField TzTimeOnlyField;
typedef StringField TzTimeStampField;

/**
 * Zero-copy, type-safe view of a stored FieldBase as a typed field T.
 *
 * FieldMap stores fields as plain FieldBase (sliced); a previous version of
 * FieldMap::getField<T>() reinterpret_cast<>ed a FieldBase* to T*, which is
 * undefined behaviour. FieldView holds a reference to the actual stored
 * FieldBase and exposes the same value-access interface as T's parent typed
 * field class, without ever pretending the underlying object is a T.
 */
template <typename T> class FieldView {
  // Map T to the value type returned by its parent typed field's getValue().
  // The branches mirror the typed field classes defined above.
  using value_type = std::conditional_t<
      std::is_base_of_v<StringField, T>,
      const std::string &,
      std::conditional_t<
          std::is_base_of_v<CharField, T>,
          char,
          std::conditional_t<
              std::is_base_of_v<DoubleField, T>,
              double,
              std::conditional_t<
                  std::is_base_of_v<Int64Field, T>,
                  int64_t,
                  std::conditional_t<
                      std::is_base_of_v<UInt64Field, T>,
                      uint64_t,
                      std::conditional_t<
                          std::is_base_of_v<IntField, T>,
                          int,
                          std::conditional_t<
                              std::is_base_of_v<BoolField, T>,
                              bool,
                              std::conditional_t<
                                  std::is_base_of_v<UtcTimeStampField, T>,
                                  UtcTimeStamp,
                                  std::conditional_t<
                                      std::is_base_of_v<UtcDateField, T>,
                                      UtcDate,
                                      std::conditional_t<
                                          std::is_base_of_v<UtcTimeOnlyField, T>,
                                          UtcTimeOnly,
                                          std::conditional_t<std::is_base_of_v<CheckSumField, T>, int, void>>>>>>>>>>>;

  static_assert(!std::is_same_v<value_type, void>, "FieldView<T>: T must derive from a typed field defined in Field.h");

public:
  explicit FieldView(const FieldBase &field)
      : m_field(field) {}

  int getTag() const { return m_field.getTag(); }
  const std::string &getString() const { return m_field.getString(); }

  /// Allows passing the view anywhere a const FieldBase& is expected
  /// (e.g. setField, operator<<).
  operator const FieldBase &() const { return m_field; }

  value_type getValue() const EXCEPT(IncorrectDataFormat) {
    if constexpr (std::is_base_of_v<StringField, T>) {
      return m_field.getString();
    } else {
      try {
        if constexpr (std::is_base_of_v<CharField, T>) {
          return CharConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<DoubleField, T>) {
          return DoubleConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<Int64Field, T>) {
          return Int64Convertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<UInt64Field, T>) {
          return UInt64Convertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<IntField, T>) {
          return IntConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<BoolField, T>) {
          return BoolConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<UtcTimeStampField, T>) {
          return UtcTimeStampConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<UtcDateField, T>) {
          return UtcDateConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<UtcTimeOnlyField, T>) {
          return UtcTimeOnlyConvertor::convert(m_field.getString());
        } else if constexpr (std::is_base_of_v<CheckSumField, T>) {
          return CheckSumConvertor::convert(m_field.getString());
        }
      } catch (FieldConvertError &) {
        throw IncorrectDataFormat(m_field.getTag(), m_field.getString());
      }
    }
  }

  /// Implicit conversion to the underlying value type, mirroring the
  /// conversion operators on the typed field classes.
  operator value_type() const { return getValue(); }

  /// String comparisons against char* / std::string / std::string_view,
  /// mirroring the free-function overloads provided for StringField.
  /// Enabled only when T derives from StringField.
  template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<StringField, U>>>
  bool operator==(std::string_view rhs) const {
    return m_field.getString() == rhs;
  }
  template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<StringField, U>>>
  bool operator!=(std::string_view rhs) const {
    return m_field.getString() != rhs;
  }
  template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<StringField, U>>>
  bool operator<(std::string_view rhs) const {
    return m_field.getString() < rhs;
  }
  template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<StringField, U>>>
  bool operator>(std::string_view rhs) const {
    return m_field.getString() > rhs;
  }
  template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<StringField, U>>>
  bool operator<=(std::string_view rhs) const {
    return m_field.getString() <= rhs;
  }
  template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<StringField, U>>>
  bool operator>=(std::string_view rhs) const {
    return m_field.getString() >= rhs;
  }

private:
  const FieldBase &m_field;
};

/// Reversed-operand string comparisons for FieldView<T> where T derives from StringField.
template <typename T, typename = std::enable_if_t<std::is_base_of_v<StringField, T>>>
inline bool operator==(std::string_view lhs, const FieldView<T> &rhs) {
  return lhs == rhs.getString();
}
template <typename T, typename = std::enable_if_t<std::is_base_of_v<StringField, T>>>
inline bool operator!=(std::string_view lhs, const FieldView<T> &rhs) {
  return lhs != rhs.getString();
}
template <typename T, typename = std::enable_if_t<std::is_base_of_v<StringField, T>>>
inline bool operator<(std::string_view lhs, const FieldView<T> &rhs) {
  return lhs < rhs.getString();
}
template <typename T, typename = std::enable_if_t<std::is_base_of_v<StringField, T>>>
inline bool operator>(std::string_view lhs, const FieldView<T> &rhs) {
  return lhs > rhs.getString();
}
template <typename T, typename = std::enable_if_t<std::is_base_of_v<StringField, T>>>
inline bool operator<=(std::string_view lhs, const FieldView<T> &rhs) {
  return lhs <= rhs.getString();
}
template <typename T, typename = std::enable_if_t<std::is_base_of_v<StringField, T>>>
inline bool operator>=(std::string_view lhs, const FieldView<T> &rhs) {
  return lhs >= rhs.getString();
}

} // namespace FIX

#define DEFINE_FIELD_CLASS_NUM(NAME, TOK, TYPE, NUM)                                                                   \
  class NAME : public TOK##Field {                                                                                     \
  public:                                                                                                              \
    static constexpr int tag = NUM;                                                                                    \
    NAME()                                                                                                             \
        : TOK##Field(NUM) {}                                                                                           \
    NAME(const TYPE &value)                                                                                            \
        : TOK##Field(NUM, value) {}                                                                                    \
  }

#define DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, TOK, TYPE, NUM)                                                           \
  class NAME : public TOK##Field {                                                                                     \
  public:                                                                                                              \
    static constexpr int tag = NUM;                                                                                    \
    NAME()                                                                                                             \
        : TOK##Field(NUM) {}                                                                                           \
    NAME(TYPE value)                                                                                                   \
        : TOK##Field(NUM, value) {}                                                                                    \
  }

#define DEFINE_FIELD_CLASS(NAME, TOK, TYPE) DEFINE_FIELD_CLASS_NUM(NAME, TOK, TYPE, FIELD::NAME)

#define DEFINE_TRIVIAL_FIELD_CLASS(NAME, TOK, TYPE) DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, TOK, TYPE, FIELD::NAME)

#define DEFINE_DEPRECATED_FIELD_CLASS(NAME, TOK, TYPE) DEFINE_FIELD_CLASS_NUM(NAME, TOK, TYPE, DEPRECATED_FIELD::NAME)

#define DEFINE_FIELD_TIMECLASS_NUM(NAME, TOK, TYPE, NUM)                                                               \
  class NAME : public TOK##Field {                                                                                     \
  public:                                                                                                              \
    static constexpr int tag = NUM;                                                                                    \
    static NAME now() { return NAME(TYPE::now()); }                                                                    \
    static NAME now(int precision) { return NAME(TYPE::now(), precision); }                                            \
    NAME()                                                                                                             \
        : TOK##Field(NUM, TYPE::now(), false) {}                                                                       \
    NAME(int precision)                                                                                                \
        : TOK##Field(NUM, TYPE::now(), precision) {}                                                                   \
    NAME(const TYPE &value)                                                                                            \
        : TOK##Field(NUM, value) {}                                                                                    \
    NAME(const TYPE &value, int precision)                                                                             \
        : TOK##Field(NUM, value, precision) {}                                                                         \
  }

#define DEFINE_FIELD_TIMECLASS(NAME, TOK, TYPE) DEFINE_FIELD_TIMECLASS_NUM(NAME, TOK, TYPE, FIELD::NAME)

#define DEFINE_DEPRECATED_FIELD_TIMECLASS(NAME, TOK, TYPE)                                                             \
  DEFINE_FIELD_TIMECLASS_NUM(NAME, TOK, TYPE, DEPRECATED_FIELD::NAME)

#define DEFINE_CHECKSUM(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, CheckSum, FIX::INT)
#define DEFINE_STRING(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::STRING)
#define DEFINE_CHAR(NAME) DEFINE_FIELD_CLASS(NAME, Char, FIX::CHAR)
#define DEFINE_PRICE(NAME) DEFINE_FIELD_CLASS(NAME, Price, FIX::PRICE)
#define DEFINE_INT(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, Int, FIX::INT)
#define DEFINE_INT64(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, Int64, FIX::INT64)
#define DEFINE_AMT(NAME) DEFINE_FIELD_CLASS(NAME, Amt, FIX::AMT)
#define DEFINE_QTY(NAME) DEFINE_FIELD_CLASS(NAME, Qty, FIX::QTY)
#define DEFINE_CURRENCY(NAME) DEFINE_FIELD_CLASS(NAME, Currency, FIX::CURRENCY)
#define DEFINE_MULTIPLEVALUESTRING(NAME) DEFINE_FIELD_CLASS(NAME, MultipleValueString, FIX::MULTIPLEVALUESTRING)
#define DEFINE_MULTIPLESTRINGVALUE(NAME) DEFINE_FIELD_CLASS(NAME, MultipleStringValue, FIX::MULTIPLESTRINGVALUE)
#define DEFINE_MULTIPLECHARVALUE(NAME) DEFINE_FIELD_CLASS(NAME, MultipleCharValue, FIX::MULTIPLECHARVALUE)
#define DEFINE_EXCHANGE(NAME) DEFINE_FIELD_CLASS(NAME, Exchange, FIX::EXCHANGE)
#define DEFINE_UTCTIMESTAMP(NAME) DEFINE_FIELD_TIMECLASS(NAME, UtcTimeStamp, FIX::UTCTIMESTAMP)
#define DEFINE_BOOLEAN(NAME) DEFINE_FIELD_CLASS(NAME, Bool, FIX::BOOLEAN)
#define DEFINE_LOCALMKTTIME(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::LOCALMKTTIME)
#define DEFINE_LOCALMKTDATE(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::LOCALMKTDATE)
#define DEFINE_DATA(NAME) DEFINE_FIELD_CLASS(NAME, Data, FIX::DATA)
#define DEFINE_FLOAT(NAME) DEFINE_FIELD_CLASS(NAME, Float, FIX::FLOAT)
#define DEFINE_PRICEOFFSET(NAME) DEFINE_FIELD_CLASS(NAME, PriceOffset, FIX::PRICEOFFSET)
#define DEFINE_MONTHYEAR(NAME) DEFINE_FIELD_CLASS(NAME, MonthYear, FIX::MONTHYEAR)
#define DEFINE_DAYOFMONTH(NAME) DEFINE_FIELD_CLASS(NAME, DayOfMonth, FIX::DAYOFMONTH)
#define DEFINE_UTCDATE(NAME) DEFINE_FIELD_CLASS(NAME, UtcDate, FIX::UTCDATE)
#define DEFINE_UTCDATEONLY(NAME) DEFINE_FIELD_CLASS(NAME, UtcDateOnly, FIX::UTCDATEONLY)
#define DEFINE_UTCTIMEONLY(NAME) DEFINE_FIELD_CLASS(NAME, UtcTimeOnly, FIX::UTCTIMEONLY)
#define DEFINE_NUMINGROUP(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, NumInGroup, FIX::NUMINGROUP)
#define DEFINE_SEQNUM(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, SeqNum, FIX::SEQNUM)
#define DEFINE_TAGNUM(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, TagNum, FIX::TAGNUM)
#define DEFINE_LENGTH(NAME) DEFINE_TRIVIAL_FIELD_CLASS(NAME, Length, FIX::LENGTH)
#define DEFINE_PERCENTAGE(NAME) DEFINE_FIELD_CLASS(NAME, Percentage, FIX::PERCENTAGE)
#define DEFINE_COUNTRY(NAME) DEFINE_FIELD_CLASS(NAME, Country, FIX::COUNTRY)
#define DEFINE_TZTIMEONLY(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::TZTIMEONLY)
#define DEFINE_TZTIMESTAMP(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::TZTIMESTAMP)
#define DEFINE_XMLDATA(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::XMLDATA)
#define DEFINE_LANGUAGE(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::LANGUAGE)
#define DEFINE_XID(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::XID)
#define DEFINE_XIDREF(NAME) DEFINE_FIELD_CLASS(NAME, String, FIX::XIDREF)

#define USER_DEFINE_STRING(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::STRING, NUM)
#define USER_DEFINE_CHAR(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Char, FIX::CHAR, NUM)
#define USER_DEFINE_PRICE(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Price, FIX::PRICE, NUM)
#define USER_DEFINE_INT(NAME, NUM) DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, Int, FIX::INT, NUM)
#define USER_DEFINE_AMT(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Amt, FIX::AMT, NUM)
#define USER_DEFINE_QTY(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Qty, FIX::QTY, NUM)
#define USER_DEFINE_CURRENCY(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Currency, FIX::CURRENCY, NUM)
#define USER_DEFINE_MULTIPLEVALUESTRING(NAME, NUM)                                                                     \
  DEFINE_FIELD_CLASS_NUM(NAME, MultipleValueString, FIX::MULTIPLEVALUESTRING, NUM)
#define USER_DEFINE_MULTIPLESTRINGVALUE(NAME, NUM)                                                                     \
  DEFINE_FIELD_CLASS_NUM(NAME, MultipleStringValue, FIX::MULTIPLESTRINGVALUE, NUM)
#define USER_DEFINE_MULTIPLECHARVALUE(NAME, NUM)                                                                       \
  DEFINE_FIELD_CLASS_NUM(NAME, MultipleCharValue, FIX::MULTIPLECHARVALUE, NUM)
#define USER_DEFINE_EXCHANGE(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Exchange, FIX::EXCHANGE, NUM)
#define USER_DEFINE_UTCTIMESTAMP(NAME, NUM) DEFINE_FIELD_TIMECLASS_NUM(NAME, UtcTimeStamp, FIX::UTCTIMESTAMP, NUM)
#define USER_DEFINE_BOOLEAN(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Bool, FIX::BOOLEAN, NUM)
#define USER_DEFINE_LOCALMKTTIME(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::LOCALMKTTIME, NUM)
#define USER_DEFINE_LOCALMKTDATE(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::STRING, NUM)
#define USER_DEFINE_DATA(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Data, FIX::DATA, NUM)
#define USER_DEFINE_FLOAT(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Float, FIX::FLOAT, NUM)
#define USER_DEFINE_PRICEOFFSET(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, PriceOffset, FIX::PRICEOFFSET, NUM)
#define USER_DEFINE_MONTHYEAR(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, MonthYear, FIX::MONTHYEAR, NUM)
#define USER_DEFINE_DAYOFMONTH(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, DayOfMonth, FIX::DAYOFMONTH, NUM)
#define USER_DEFINE_UTCDATE(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, UtcDate, FIX::UTCDATE, NUM)
#define USER_DEFINE_UTCDATEONLY(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, UtcDateOnly, FIX::UTCDATEONLY, NUM)
#define USER_DEFINE_UTCTIMEONLY(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, UtcTimeOnly, FIX::UTCTIMEONLY, NUM)
#define USER_DEFINE_NUMINGROUP(NAME, NUM) DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, NumInGroup, FIX::NUMINGROUP, NUM)
#define USER_DEFINE_SEQNUM(NAME, NUM) DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, SeqNum, FIX::SEQNUM, NUM)
#define USER_DEFINE_TAGNUM(NAME, NUM) DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, TagNum, FIX::TAGNUM, NUM)
#define USER_DEFINE_LENGTH(NAME, NUM) DEFINE_TRIVIAL_FIELD_CLASS_NUM(NAME, Length, FIX::LENGTH, NUM)
#define USER_DEFINE_PERCENTAGE(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Percentage, FIX::PERCENTAGE, NUM)
#define USER_DEFINE_COUNTRY(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, Country, FIX::COUNTRY, NUM)
#define USER_DEFINE_TZTIMEONLY(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::TZTIMEONLY, NUM)
#define USER_DEFINE_TZTIMESTAMP(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::TZTIMESTAMP, NUM)
#define USER_DEFINE_XMLDATA(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::XMLDATA, NUM)
#define USER_DEFINE_LANGUAGE(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::LANGUAGE, NUM)
#define USER_DEFINE_XID(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::XID, NUM)
#define USER_DEFINE_XIDREF(NAME, NUM) DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::XIDREF, NUM)

#endif
