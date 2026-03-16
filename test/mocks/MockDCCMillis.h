#ifndef MOCK_DCCMILLIS_H
#define MOCK_DCCMILLIS_H

#include <DCCMillis.h>
#include "millis.h"

class MockDCCMillis : public DCCExController::DCCMillis {
public:
  unsigned long millis() const override { return getMillis(); }
};

#endif // MOCK_DCCMILLIS_H
