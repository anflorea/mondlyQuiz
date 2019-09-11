#include "BaseModel.h"

bool BaseModel::operator ==(const BaseModel &other) const
{
    if (m_ref_id == other.get_ref_id()) {
        return true;
    } else {
        return false;
    }
}
