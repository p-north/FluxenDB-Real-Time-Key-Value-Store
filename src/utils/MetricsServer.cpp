#include "../../include/Metrics.h"
#include "../../include/httplib.h"

void startMetricsServer()
{
    httplib::Server svr;

    svr.Get("/metrics", [](const httplib::Request &, httplib::Response &res)
            { res.set_content(
                  Metrics::getInstance().exportMetrics(), "text/plain"); });

    svr.listen("0.0.0.0", PORT);
}