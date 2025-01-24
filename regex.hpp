

#include <regex>
#include <ranges>
#include <string_view>


namespace regex {
  template<typename SearchString>
  auto iter(SearchString&& searchString, const std::regex& regex) {
    static_assert(!std::is_same_v<SearchString, std::string>, "Cannot use regex::iter() on temporary std::string due to lifetime issues!");
    using iterator_type = decltype(std::begin(searchString));
    return std::ranges::subrange{
      std::regex_iterator<iterator_type>(std::begin(searchString), std::end(searchString), regex),
      std::regex_iterator<iterator_type>()
    };
  }

  /** Concept, which ensures, that the given match results type matches the search string's iterator type
   */
  template<typename MatchResults, typename SearchString>
  concept IsMatchFor = requires(SearchString&& s) {
    requires std::same_as<std::remove_reference_t<MatchResults>, std::match_results<decltype(std::cbegin(s))>>;
  };

  namespace impl {
    template<typename SearchString, IsMatchFor<SearchString> MatchResults>
    struct match_result {
      match_result(SearchString&& searchString) : searchString(std::forward<SearchString>(searchString)), matchResults() {}
      match_result(SearchString&& searchString, MatchResults&& matchResults) : 
        searchString(std::forward<SearchString>(searchString)), matchResults(std::forward<MatchResults>(matchResults)) {}

      explicit operator bool() const { return matched; }
      auto& operator[](int i) const { return matchResults[i]; }

      SearchString searchString;
      MatchResults matchResults;
      bool matched;
    };
  }

  /** matches the given search string (which can also be a temporary std::string, a const char*, or std::string_view)
   *  and stores the results in the passed matchResults. A temporary std::string will be copied into the result object.
   */
  template<typename SearchString, IsMatchFor<SearchString> MatchResults>
  auto match(SearchString&& searchString, MatchResults&& matchResults, const std::regex& regex, std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    impl::match_result<SearchString, MatchResults> result(std::forward<SearchString>(searchString), std::forward<MatchResults>(matchResults));
    result.matched = std::regex_match(std::cbegin(result.searchString), std::cend(result.searchString), result.matchResults, regex, flags);
    return result;
  }

  /** Overload with implicitly defined match_result
   */
  template<typename SearchString>
  auto match(SearchString&& searchString, const std::regex& regex, std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    using iterator_type = decltype(std::cbegin(searchString));
    impl::match_result<SearchString, std::match_results<iterator_type>> result(std::forward<SearchString>(searchString));
    result.matched = std::regex_match(std::cbegin(result.searchString), std::cend(result.searchString), result.matchResults, regex, flags);
    return result;
  }


  /** searches for a match in given search string (which can also be a temporary std::string, a const char*, or std::string_view)
   *  and stores the results in the passed matchResults. A temporary std::string will be copied into the result object.
   */
  template<typename SearchString, IsMatchFor<SearchString> MatchResults>
  auto search(SearchString&& searchString, MatchResults&& matchResults, const std::regex& regex, std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    impl::match_result<SearchString, MatchResults> result(std::forward<SearchString>(searchString), std::forward<MatchResults>(matchResults));
    result.matched = std::regex_search(std::cbegin(result.searchString), std::cend(result.searchString), result.matchResults, regex, flags);
    return result;
  }

  /** Overload with implicitly defined match_result
   */
  template<typename SearchString>
  auto search(SearchString&& searchString, const std::regex& regex, std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    using iterator_type = decltype(std::cbegin(searchString));
    impl::match_result<SearchString, std::match_results<iterator_type>> result(std::forward<SearchString>(searchString));
    result.matched = std::regex_search(std::cbegin(result.searchString), std::cend(result.searchString), result.matchResults, regex, flags);
    return result;
  }
}