#pragma once
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class IssueDTO : public oatpp::DTO {
    DTO_INIT(IssueDTO, DTO)

    DTO_FIELD(Int32, id);
    DTO_FIELD(String, title);
    DTO_FIELD(String, description);
    DTO_FIELD(String, priority);
    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)
