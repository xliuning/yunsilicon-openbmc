#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <xyz/openbmc_project/Sensor/Threshold/Critical/server.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Warning/server.hpp>
#include <xyz/openbmc_project/State/Decorator/OperationalStatus/server.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace
{

constexpr const char* sensorInterface =
    "xyz.openbmc_project.Sensor.Value";

constexpr const char* propertiesInterface =
    "org.freedesktop.DBus.Properties";

constexpr const char* hwmonService =
    "xyz.openbmc_project.Hwmon-PECI_CPU.Hwmon1";

constexpr const char* warningThresholdInterface =
    "xyz.openbmc_project.Sensor.Threshold.Warning";

constexpr const char* criticalThresholdInterface =
    "xyz.openbmc_project.Sensor.Threshold.Critical";

constexpr const char* operationalStatusInterface =
    "xyz.openbmc_project.State.Decorator.OperationalStatus";


using Warning =
    sdbusplus::server::xyz::openbmc_project::sensor::threshold::Warning;

using Critical =
    sdbusplus::server::xyz::openbmc_project::sensor::threshold::Critical;

using OperationalStatus =
    sdbusplus::server::xyz::openbmc_project::state::decorator::
        OperationalStatus;


/*
 * -------------------------------------------------------------
 * Environment helpers
 * -------------------------------------------------------------
 */

std::string getEnv(
    const char* name,
    const std::string& defaultValue)
{
    const char* value = std::getenv(name);

    if (value == nullptr || *value == '\0')
    {
        return defaultValue;
    }

    return value;
}


double getEnvDouble(
    const char* name,
    double defaultValue)
{
    const char* value = std::getenv(name);

    if (value == nullptr || *value == '\0')
    {
        return defaultValue;
    }

    try
    {
        return std::stod(value);
    }
    catch (...)
    {
        std::cerr
            << "Invalid "
            << name
            << ": "
            << value
            << ", using default "
            << defaultValue
            << "\n";

        return defaultValue;
    }
}


int getEnvInt(
    const char* name,
    int defaultValue)
{
    const char* value = std::getenv(name);

    if (value == nullptr || *value == '\0')
    {
        return defaultValue;
    }

    try
    {
        return std::stoi(value);
    }
    catch (...)
    {
        std::cerr
            << "Invalid "
            << name
            << ": "
            << value
            << ", using default "
            << defaultValue
            << "\n";

        return defaultValue;
    }
}


/*
 * Threshold values in the configuration are m°C.
 *
 * Example:
 *
 *     WARNHI_temp11=80000
 *
 * becomes:
 *
 *     80.0 °C
 */
double getEnvMilliDegree(
    const char* name,
    double defaultValue)
{
    const char* value = std::getenv(name);

    if (value == nullptr || *value == '\0')
    {
        return defaultValue;
    }

    try
    {
        const double milliDegrees =
            std::stod(value);

        return milliDegrees / 1000.0;
    }
    catch (...)
    {
        std::cerr
            << "Invalid "
            << name
            << ": "
            << value
            << ", using default "
            << defaultValue
            << " C\n";

        return defaultValue;
    }
}


std::vector<std::string> split(
    const std::string& input)
{
    std::vector<std::string> result;

    std::istringstream stream(input);
    std::string item;

    while (stream >> item)
    {
        result.emplace_back(item);
    }

    return result;
}


std::string sensorPath(
    const std::string& sensorName)
{
    return "/xyz/openbmc_project/sensors/temperature/" +
           sensorName;
}

} // namespace


class CpuTempAggregator :
    public std::enable_shared_from_this<CpuTempAggregator>
{
  public:

    CpuTempAggregator(
        const std::shared_ptr<sdbusplus::asio::connection>& connection,
        sdbusplus::asio::object_server& objectServer,
        const std::vector<std::string>& coreSensors,
        const std::string& sensorName,
        double minValue,
        double maxValue,
        double warningLow,
        double warningHigh,
        double criticalLow,
        double criticalHigh,
        int updateInterval) :
        connection(connection),
        objectServer(objectServer),
        coreSensors(coreSensors),
        sensorName(sensorName),
        minValue(minValue),
        maxValue(maxValue),
        warningLow(warningLow),
        warningHigh(warningHigh),
        criticalLow(criticalLow),
        criticalHigh(criticalHigh),
        updateInterval(updateInterval)
    {
    }


    void start()
    {
        const std::string path =
            sensorPath(sensorName);


        /*
         * =========================================================
         * Sensor.Value
         * =========================================================
         */

        sensorInterfaceObject =
            objectServer.add_interface(
                path,
                sensorInterface);

        sensorInterfaceObject->register_property(
            "Value",
            0.0);

        sensorInterfaceObject->register_property(
            "MinValue",
            minValue);

        sensorInterfaceObject->register_property(
            "MaxValue",
            maxValue);

        sensorInterfaceObject->register_property(
            "Unit",
            std::string_view(
                "xyz.openbmc_project.Sensor.Value.Unit.DegreesC"));

        sensorInterfaceObject->initialize();


        /*
         * =========================================================
         * Standard Warning interface
         * =========================================================
         *
         * IMPORTANT:
         *
         * Do NOT manually register:
         *
         *     WarningLow
         *     WarningHigh
         *     WarningAlarmLow
         *     WarningAlarmHigh
         *
         * The generated Warning server owns these properties.
         */

        warningInterfaceObject =
            std::make_shared<Warning>(
                *connection,
                path.c_str());

        warningInterfaceObject->warningLow(
            warningLow);

        warningInterfaceObject->warningHigh(
            warningHigh);

        warningInterfaceObject->warningAlarmLow(
            false);

        warningInterfaceObject->warningAlarmHigh(
            false);


        /*
         * =========================================================
         * Standard Critical interface
         * =========================================================
         */

        criticalInterfaceObject =
            std::make_shared<Critical>(
                *connection,
                path.c_str());

        criticalInterfaceObject->criticalLow(
            criticalLow);

        criticalInterfaceObject->criticalHigh(
            criticalHigh);

        criticalInterfaceObject->criticalAlarmLow(
            false);

        criticalInterfaceObject->criticalAlarmHigh(
            false);


        /*
         * =========================================================
         * OperationalStatus
         * =========================================================
         */

        operationalStatusInterfaceObject =
            std::make_shared<OperationalStatus>(
                *connection,
                path.c_str());

        /*
         * Sensor is initially not functional until the first
         * successful CPU temperature aggregation.
         */
        operationalStatusInterfaceObject->functional(
            false);


        /*
         * =========================================================
         * Print configuration
         * =========================================================
         */

        std::cout
            << "========================================\n"
            << "CPU temperature aggregator started\n"
            << "========================================\n";

        std::cout
            << "Sensor: "
            << sensorName
            << "\n";

        std::cout
            << "Object path: "
            << path
            << "\n";

        std::cout
            << "MinValue: "
            << minValue
            << " C\n";

        std::cout
            << "MaxValue: "
            << maxValue
            << " C\n";

        std::cout
            << "WarningLow: "
            << warningLow
            << " C\n";

        std::cout
            << "WarningHigh: "
            << warningHigh
            << " C\n";

        std::cout
            << "CriticalLow: "
            << criticalLow
            << " C\n";

        std::cout
            << "CriticalHigh: "
            << criticalHigh
            << " C\n";

        std::cout
            << "Update interval: "
            << updateInterval
            << " seconds\n";

        std::cout
            << "Aggregation: MAX\n";

        std::cout
            << "CPU core sensors:\n";

        for (const auto& sensor : coreSensors)
        {
            std::cout
                << "  "
                << sensor
                << "\n";
        }

        std::cout
            << "========================================\n";


        /*
         * Start first update.
         */
        update();
    }


  private:

    std::shared_ptr<sdbusplus::asio::connection>
        connection;

    sdbusplus::asio::object_server&
        objectServer;


    /*
     * Sensor.Value
     */
    std::shared_ptr<sdbusplus::asio::dbus_interface>
        sensorInterfaceObject;


    /*
     * Standard generated interfaces.
     */
    std::shared_ptr<Warning>
        warningInterfaceObject;

    std::shared_ptr<Critical>
        criticalInterfaceObject;

    std::shared_ptr<OperationalStatus>
        operationalStatusInterfaceObject;


    /*
     * Configuration.
     */
    std::vector<std::string>
        coreSensors;

    std::string
        sensorName;

    double
        minValue;

    double
        maxValue;

    double
        warningLow;

    double
        warningHigh;

    double
        criticalLow;

    double
        criticalHigh;

    int
        updateInterval;


    /*
     * =============================================================
     * Update CPU temperature
     * =============================================================
     */

    void update()
    {
        auto self =
            shared_from_this();

        if (coreSensors.empty())
        {
            std::cerr
                << "No CPU core sensors configured\n";

            setSensorFunctional(false);

            scheduleNextUpdate();

            return;
        }


        /*
         * Maximum temperature among all valid CPU cores.
         */
        auto maximum =
            std::make_shared<double>(
                -std::numeric_limits<double>::infinity());


        /*
         * Number of valid CPU sensors.
         */
        auto validCount =
            std::make_shared<int>(0);


        readSensor(
            0,
            maximum,
            validCount,
            [self, maximum, validCount]()
            {
                /*
                 * No valid CPU sensor.
                 */
                if (*validCount == 0)
                {
                    std::cerr
                        << "No valid CPU core temperature "
                           "available\n";

                    self->setSensorFunctional(false);

                    self->scheduleNextUpdate();

                    return;
                }


                /*
                 * CPU0_TEMP = MAX(valid CPU core temperature)
                 */
                double value =
                    *maximum;


                /*
                 * Clamp to Sensor.Value range.
                 */
                if (value < self->minValue)
                {
                    value =
                        self->minValue;
                }

                if (value > self->maxValue)
                {
                    value =
                        self->maxValue;
                }


                /*
                 * Update D-Bus Sensor.Value.
                 */
                self->sensorInterfaceObject->set_property(
                    "Value",
                    value);


                /*
                 * Sensor is functional.
                 */
                self->setSensorFunctional(true);


                /*
                 * Update threshold alarms.
                 */
                self->updateThresholds(
                    value);


                std::cout
                    << "CPU0_TEMP = "
                    << value
                    << " C"
                    << " (valid cores="
                    << *validCount
                    << ")\n";


                self->scheduleNextUpdate();
            });
    }


    /*
     * =============================================================
     * Read one CPU core sensor
     * =============================================================
     */

    void readSensor(
        std::size_t index,
        const std::shared_ptr<double>& maximum,
        const std::shared_ptr<int>& validCount,
        const std::function<void()>& complete)
    {
        if (index >= coreSensors.size())
        {
            complete();

            return;
        }


        const std::string path =
            sensorPath(
                coreSensors[index]);


        auto self =
            shared_from_this();


        connection->async_method_call(
            [self,
             index,
             maximum,
             validCount,
             complete](
                const boost::system::error_code& ec,
                const std::variant<double>& value)
            {
                if (ec)
                {
                    std::cerr
                        << "Failed to read "
                        << self->coreSensors[index]
                        << ": "
                        << ec.message()
                        << "\n";
                }
                else
                {
                    const double temperature =
                        std::get<double>(value);


                    if (std::isfinite(temperature))
                    {
                        *maximum =
                            std::max(
                                *maximum,
                                temperature);

                        ++(*validCount);


                        std::cout
                            << "  "
                            << self->coreSensors[index]
                            << " = "
                            << temperature
                            << " C\n";
                    }
                    else
                    {
                        std::cerr
                            << "Invalid temperature from "
                            << self->coreSensors[index]
                            << ": "
                            << temperature
                            << "\n";
                    }
                }


                /*
                 * Continue with next CPU core.
                 */
                self->readSensor(
                    index + 1,
                    maximum,
                    validCount,
                    complete);
            },
            hwmonService,
            path,
            propertiesInterface,
            "Get",
            sensorInterface,
            "Value");
    }


    /*
     * =============================================================
     * OperationalStatus
     * =============================================================
     */

    void setSensorFunctional(
        bool functional)
    {
        if (!operationalStatusInterfaceObject)
        {
            return;
        }

        operationalStatusInterfaceObject->functional(
            functional);
    }


    /*
     * =============================================================
     * Threshold handling
     * =============================================================
     *
     * Alarm signals are generated ONLY when the alarm state
     * changes.
     *
     * This is important:
     *
     *     80 -> 81 -> 82 -> 83
     *
     * must NOT generate four WarningHigh Asserted events.
     *
     * Only:
     *
     *     79 -> 81
     *
     * generates:
     *
     *     WarningHighAlarmAsserted
     *
     * And:
     *
     *     81 -> 79
     *
     * generates:
     *
     *     WarningHighAlarmDeasserted
     */

    void updateThresholds(
        double value)
    {
        if (!warningInterfaceObject ||
            !criticalInterfaceObject)
        {
            return;
        }


        /*
         * ---------------------------------------------------------
         * Warning High
         * ---------------------------------------------------------
         */

        const bool oldWarningHigh =
            warningInterfaceObject->warningAlarmHigh();

        const bool newWarningHigh =
            value >=
            warningInterfaceObject->warningHigh();


        if (oldWarningHigh != newWarningHigh)
        {
            warningInterfaceObject->warningAlarmHigh(
                newWarningHigh);

            if (newWarningHigh)
            {
                warningInterfaceObject->
                    warningHighAlarmAsserted(
                        value);
            }
            else
            {
                warningInterfaceObject->
                    warningHighAlarmDeasserted(
                        value);
            }

            std::cout
                << "CPU0_TEMP WarningHigh "
                << (newWarningHigh
                        ? "ASSERTED"
                        : "DEASSERTED")
                << " value="
                << value
                << "\n";
        }


        /*
         * ---------------------------------------------------------
         * Warning Low
         * ---------------------------------------------------------
         */

        const bool oldWarningLow =
            warningInterfaceObject->warningAlarmLow();

        const bool newWarningLow =
            value <=
            warningInterfaceObject->warningLow();


        if (oldWarningLow != newWarningLow)
        {
            warningInterfaceObject->warningAlarmLow(
                newWarningLow);

            if (newWarningLow)
            {
                warningInterfaceObject->
                    warningLowAlarmAsserted(
                        value);
            }
            else
            {
                warningInterfaceObject->
                    warningLowAlarmDeasserted(
                        value);
            }

            std::cout
                << "CPU0_TEMP WarningLow "
                << (newWarningLow
                        ? "ASSERTED"
                        : "DEASSERTED")
                << " value="
                << value
                << "\n";
        }


        /*
         * ---------------------------------------------------------
         * Critical High
         * ---------------------------------------------------------
         */

        const bool oldCriticalHigh =
            criticalInterfaceObject->criticalAlarmHigh();

        const bool newCriticalHigh =
            value >=
            criticalInterfaceObject->criticalHigh();


        if (oldCriticalHigh != newCriticalHigh)
        {
            criticalInterfaceObject->criticalAlarmHigh(
                newCriticalHigh);

            if (newCriticalHigh)
            {
                criticalInterfaceObject->
                    criticalHighAlarmAsserted(
                        value);
            }
            else
            {
                criticalInterfaceObject->
                    criticalHighAlarmDeasserted(
                        value);
            }

            std::cout
                << "CPU0_TEMP CriticalHigh "
                << (newCriticalHigh
                        ? "ASSERTED"
                        : "DEASSERTED")
                << " value="
                << value
                << "\n";
        }


        /*
         * ---------------------------------------------------------
         * Critical Low
         * ---------------------------------------------------------
         */

        const bool oldCriticalLow =
            criticalInterfaceObject->criticalAlarmLow();

        const bool newCriticalLow =
            value <=
            criticalInterfaceObject->criticalLow();


        if (oldCriticalLow != newCriticalLow)
        {
            criticalInterfaceObject->criticalAlarmLow(
                newCriticalLow);

            if (newCriticalLow)
            {
                criticalInterfaceObject->
                    criticalLowAlarmAsserted(
                        value);
            }
            else
            {
                criticalInterfaceObject->
                    criticalLowAlarmDeasserted(
                        value);
            }

            std::cout
                << "CPU0_TEMP CriticalLow "
                << (newCriticalLow
                        ? "ASSERTED"
                        : "DEASSERTED")
                << " value="
                << value
                << "\n";
        }
    }


    /*
     * =============================================================
     * Schedule next update
     * =============================================================
     */

    void scheduleNextUpdate()
    {
        auto timer =
            std::make_shared<
                boost::asio::steady_timer>(
                    connection->get_io_context());


        timer->expires_after(
            std::chrono::seconds(
                updateInterval));


        auto self =
            shared_from_this();


        timer->async_wait(
            [self, timer](
                const boost::system::error_code& ec)
            {
                if (ec)
                {
                    return;
                }

                self->update();
            });
    }
};


int main()
{
    try
    {
        /*
         * =========================================================
         * Read configuration
         * =========================================================
         */

        const std::string coreSensorString =
            getEnv(
                "CORE_SENSORS",
                "CPU_Core0 "
                "CPU_Core1 "
                "CPU_Core2 "
                "CPU_Core4 "
                "CPU_Core5 "
                "CPU_Core6 "
                "CPU_Core8 "
                "CPU_Core9");


        const std::string sensorName =
            getEnv(
                "SENSOR_NAME",
                "CPU0_TEMP");


        const double minValue =
            getEnvDouble(
                "MIN_VALUE",
                -40.0);


        const double maxValue =
            getEnvDouble(
                "MAX_VALUE",
                125.0);


        /*
         * =========================================================
         * Thresholds
         * =========================================================
         *
         * Configuration format:
         *
         *     WARNLO_temp11=-20000
         *     WARNHI_temp11=80000
         *     CRITLO_temp11=-40000
         *     CRITHI_temp11=95000
         *
         * Unit:
         *
         *     milli-degree Celsius
         *
         * Converted to:
         *
         *     -20 C
         *      80 C
         *     -40 C
         *      95 C
         */

        const double warningLow =
            getEnvMilliDegree(
                "WARNLO_temp11",
                -20.0);


        const double warningHigh =
            getEnvMilliDegree(
                "WARNHI_temp11",
                80.0);


        const double criticalLow =
            getEnvMilliDegree(
                "CRITLO_temp11",
                -40.0);


        const double criticalHigh =
            getEnvMilliDegree(
                "CRITHI_temp11",
                95.0);


        /*
         * =========================================================
         * Update interval
         * =========================================================
         */

        const int updateInterval =
            getEnvInt(
                "UPDATE_INTERVAL",
                2);


        /*
         * =========================================================
         * Parse CPU core list
         * ========================================================= */

        const auto coreSensors =
            split(coreSensorString);


        /*
         * =========================================================
         * Validate configuration
         * ========================================================= */

        if (coreSensors.empty())
        {
            std::cerr
                << "CORE_SENSORS is empty\n";

            return 1;
        }


        if (updateInterval <= 0)
        {
            std::cerr
                << "UPDATE_INTERVAL must be "
                   "greater than 0\n";

            return 1;
        }


        if (maxValue <= minValue)
        {
            std::cerr
                << "MAX_VALUE must be greater "
                   "than MIN_VALUE\n";

            return 1;
        }


        if (warningLow >= warningHigh)
        {
            std::cerr
                << "WarningLow must be less "
                   "than WarningHigh\n";

            return 1;
        }


        if (criticalLow >= criticalHigh)
        {
            std::cerr
                << "CriticalLow must be less "
                   "than CriticalHigh\n";

            return 1;
        }

/*
 * =========================================================
 * Start D-Bus
 * ========================================================= */

boost::asio::io_context io;

auto connection =
    std::make_shared<
        sdbusplus::asio::connection>(
            io);

connection->request_name(
    "xyz.openbmc_project.CpuTempAggregator");

sdbusplus::asio::object_server objectServer(
    connection);

/*
 * =========================================================
 * ObjectManager for IPMI / GetManagedObjects
 * =========================================================
 *
 * phosphor-ipmi-host::getSensorMap() calls:
 *
 *   GetManagedObjects(
 *       /xyz/openbmc_project/sensors)
 *
 * Therefore this service must expose an
 * org.freedesktop.DBus.ObjectManager at this path.
 */
objectServer.add_manager(
    "/xyz/openbmc_project/sensors");

/*
 * =========================================================
 * Create aggregator
 * ========================================================= */

auto aggregator =
    std::make_shared<CpuTempAggregator>(
        connection,
        objectServer,
        coreSensors,
        sensorName,
        minValue,
        maxValue,
        warningLow,
        warningHigh,
        criticalLow,
        criticalHigh,
        updateInterval);

aggregator->start();

/*
 * =========================================================
 * Run
 * ========================================================= */

io.run();

return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "CPU temperature aggregator failed: "
            << e.what()
            << "\n";

        return 1;
    }
}
