#pragma once

namespace databento {
static constexpr auto kApiVersion = 0;
static constexpr auto kApiVersionStr = "0";

// This is not necessarily a comprehensive list of available datasets. Please
// use `Historical.MetadataListDatasets` to retrieve an up-to-date list.
namespace dataset {
static constexpr auto kGlbxMdp3 = "GLBX.MDP3";
static constexpr auto kXnasItch = "XNAS.ITCH";
}  // namespace dataset
}  // namespace databento
