// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <dxfg_endpoint.h>

#include "internal/CEntryPointErrors.hpp"
#include "internal/Common.hpp"
#include "internal/Isolate.hpp"

#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include <fmt/format.h>
#include <fmt/std.h>

namespace dxfcpp {
struct DXEndpoint {
    /**
     * Defines property for endpoint name that is used to distinguish multiple endpoints
     * in the same process in logs and in other diagnostic means.
     * Use Builder::withProperty(const std::string&, const std::string&) method.
     * This property is also changed by Builder::withName(const std::string&) method.
     */
    static const std::string NAME_PROPERTY;

    /**
     * Defines path to a file with properties for an endpoint with role Role::FEED or Role::ON_DEMAND_FEED.
     * By default, properties a loaded from a path resource named "dxfeed.properties".
     *
     * @see Builder::withProperty(const std::string&, const std::string&)
     */
    static const std::string DXFEED_PROPERTIES_PROPERTY;

    /**
     * Defines default connection address for an endpoint with role Role::FEED or Role::ON_DEMAND_FEED.
     * Connection is established to this address by role Role::FEED as soon as endpoint is created, while role
     * Role::ON_DEMAND_FEED waits until OnDemandService::replay(Date, double) is invoked before connecting.
     *
     * By default, without this property, connection is not established until ::connect(const std::string&) is invoked.
     *
     * Credentials for access to premium services may be configured with
     * ::DXFEED_USER_PROPERTY and ::DXFEED_PASSWORD_PROPERTY.
     *
     * @see Builder::withProperty(const std::string&, const std::string&)
     */
    static const std::string DXFEED_ADDRESS_PROPERTY;

    /**
     * Defines default user name for an endpoint with role Role::FEED or Role::ON_DEMAND_FEED.
     *
     * @see ::user(const std::string&)
     */
    static const std::string DXFEED_USER_PROPERTY;

    /**
     * Defines default password for an endpoint with role Role::FEED or Role::ON_DEMAND_FEED.
     *
     * @see ::password(const std::string&)
     */
    static const std::string DXFEED_PASSWORD_PROPERTY;

    /**
     * Defines thread pool size for an endpoint with role Role::FEED.
     * By default, the thread pool size is equal to the number of available processors.
     * @see Builder::withProperty(const std::string&, const std::string&)
     */
    static const std::string DXFEED_THREAD_POOL_SIZE_PROPERTY;

    /**
     * Defines data aggregation period an endpoint with role Role::FEED that
     * limits the rate of data notifications. For example, setting the value of this property
     * to "0.1s" limits notification to once every 100ms (at most 10 per second).
     * @see Builder::withProperty(const std::string&, const std::string&)
     */
    static const std::string DXFEED_AGGREGATION_PERIOD_PROPERTY;

    /**
     * Set this property to `true` to turns on wildcard support.
     * By default, the endpoint does not support wildcards. This property is needed for
     * WildcardSymbol support and for the use of "tape:..." address in DXPublisher.
     */
    static const std::string DXFEED_WILDCARD_ENABLE_PROPERTY;

    /**
     * Defines path to a file with properties for an endpoint with role Role::PUBLISHER.
     * By default, properties a loaded from a classpath resource named "dxpublisher.properties".
     * @see Builder::withProperty(const std::string&, const std::string&)
     */
    static const std::string DXPUBLISHER_PROPERTIES_PROPERTY;

    /**
     * Defines default connection address for an endpoint with role Role::PUBLISHER.
     * Connection is established to this address as soon as endpoint is created.
     * By default, connection is not established until ::connect(const std::string&) is invoked.
     * @see Builder::withProperty(const std::string&, const std::string&)
     */
    static const std::string DXPUBLISHER_ADDRESS_PROPERTY;

    /**
     * Defines thread pool size for an endpoint with role Role::PUBLISHER.
     * By default, the thread pool size is equal to the number of available processors.
     * @see Builder#withProperty(const std::string&, const std::string&)
     */
    static const std::string DXPUBLISHER_THREAD_POOL_SIZE_PROPERTY;

    /**
     * Set this property to `true` to enable EventType::getEventTime() support.
     * By default, the endpoint does not support event time.
     *
     * <p>The event time is available only when the corresponding DXEndpoint is created with this property and
     * the data source has embedded event times. This is typically true only for data events
     * that are read from historical tape files and from OnDemandService.
     * Events that are coming from a network connections do not have an embedded event time information and
     * event time is not available for them anyway.
     *
     * <p>Use this property if you need to work with historical data coming from files
     * or from OnDemandService or writing data with times to file via DXPublisher using "tape:..." address.
     */
    static const std::string DXENDPOINT_EVENT_TIME_PROPERTY;

    /**
     * Set this property to  to store all LastingEvent and IndexedEvent events even when there is no subscription on them.
     * By default, the endpoint stores only events from subscriptions. It works in the same way both
     * for DXFeed and DXPublisher.
     *
     * <p>Use this property with extreme care,
     * since API does not currently provide any means to remove those events from the storage and there might
     * be an effective memory leak if the spaces of symbols on which events are published grows without bound.
     */
    static const std::string DXENDPOINT_STORE_EVERYTHING_PROPERTY;

    /**
     * Set this property to `true` to turn on nanoseconds precision business time.
     * By default, this feature is turned off.
     * Business time in most events is available with
     * millisecond precision by default, while Quote events business Quote::getTime() is available with seconds precision.
     *
     * <p>This method provides a higher-level control than turning on individual properties that are responsible
     * for nano-time via ::DXSCHEME_ENABLED_PROPERTY_PREFIX. The later can be used to override of fine-time
     * nano-time support for individual fields. Setting this property to `true` is essentially
     * equivalent to setting<br>
     * <pre><tt>
     * dxscheme.enabled.Sequence=*
     * dxscheme.enabled.TimeNanoPart=*
     * </tt></pre>
     */
    static const std::string DXSCHEME_NANO_TIME_PROPERTY;

    /**
     * Defines whether a specified field from the scheme should be enabled instead of it's default behaviour.
     * Use it according to following format:
     *
     * <p>`dxscheme.enabled.<field_property_name>=<event_name_mask_glob>`
     *
     * <p>For example, <b>`dxscheme.enabled.TimeNanoPart=Trade`</b> enables `NanoTimePart` internal field
     * only in Trade events.
     *
     * <p>There is a shortcut for turning on nano-time support using ::DXSCHEME_NANO_TIME_PROPERTY.
     */
    static const std::string DXSCHEME_ENABLED_PROPERTY_PREFIX;
};
} // namespace dxfcpp