#include <string_view>
#include <string>
#include <algorithm>

/** An improved version of the previous string_view::split() function, which didn't work on temporary std::string
 *  values. If a temporary std::string is passed in this version will move it into the internal range and keep it valid for the
 *  duration of the iteration.
 */
namespace common {
  namespace impl {
    template<typename Source, typename SplitByType> requires std::constructible_from<std::string_view, Source>
    struct StringViewSplit {
      StringViewSplit(Source&& source, SplitByType splitBy) : source(std::forward<Source>(source)), splitBy(splitBy) {}

      struct iterator {
        using element_type = std::string_view;
        using reference_type = const std::string_view&;
        using pointer_type = const std::string_view*;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;

        iterator(std::string_view source, SplitByType splitBy) : source(source), splitBy(splitBy) {
          ++(*this); // position onto first split
        }

        iterator& operator++() {
          if (separatorEndPos == std::string_view::npos) {
            completed = true;
            return *this;
          }

          auto pos = source.find(splitBy, separatorEndPos);
          if (pos == std::string_view::npos) {
            // No more separators found
            subMatch = source.substr(separatorEndPos);
            separatorEndPos = pos;
          } else {
            // Found another separator
            subMatch = source.substr(separatorEndPos, pos - separatorEndPos);
            separatorEndPos = pos;
            if constexpr (std::is_same_v<SplitByType, char>) {
              ++separatorEndPos; // increment past the separator char
            } else {
              separatorEndPos += std::string_view(splitBy).size(); // string_view or char* separator
            }
          }

          return *this;
        }

        element_type operator*() const { return subMatch; }

        struct sentinel {};

        bool operator==(sentinel) const { return completed; }
        bool operator!=(sentinel) const { return !completed; }

        std::string_view source;
        std::string_view subMatch;
        SplitByType splitBy;
        size_t separatorEndPos = 0;
        bool completed = false;
      };

      auto begin() const { return iterator(source, splitBy); }
      auto end() const { return iterator::sentinel(); }

      SplitByType splitBy;
      Source source; // Usually one of std::string, std::string&, std::string_view, std::string_view&, but also const char* is possible
    };
  }



  template<typename Source> requires std::constructible_from<std::string_view, Source>
  auto split(Source&& source, char splitChar) {
    return impl::StringViewSplit<Source, char>(std::forward<Source>(source), splitChar);
  }

  template<typename Source> requires std::constructible_from<std::string_view, Source>
  auto split(Source&& source, std::string_view splitString) {
    return impl::StringViewSplit<Source, std::string_view>(std::forward<Source>(source), splitString);
  }

  /** Helper function for the common task of splitting a string into two by a separating character
   *  The split will be performed at the first occurrence of splitChar if multiple occurrences exist.
   *  If no split character exists, then the [source, ""] is returned
   */
  std::pair<std::string_view, std::string_view> split2(std::string_view source, char splitChar) {
    auto splitPos = std::find(source.begin(), source.end(), splitChar);

    return std::make_pair(
      std::string_view(source.begin(), splitPos),
      std::string_view(splitPos != source.end() ? splitPos + 1 : source.end(), source.end())
    );
  }

  /** Helper function for the common task of splitting a string into two by a separating string
   *  The split will be performed at the first occurrence of splitString if multiple occurrences exist.
   *  If no split character exists, then the [source, ""] is returned
   */
  std::pair<std::string_view, std::string_view> split2(std::string_view source, std::string_view splitString) {
    auto splitPos = std::search(source.begin(), source.end(), splitString.begin(), splitString.end());

    return std::make_pair(
      std::string_view(source.begin(), splitPos),
      std::string_view(splitPos != source.end() ? splitPos + splitString.length() : source.end(), source.end())
    );
  }
}

