import esphome.config_validation as cv
from esphome.loader import get_component, ComponentManifest
from pathlib import Path

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

__AVAILABLE_DRIVERS = {
    f.stem.removeprefix("driver_") for f in Path(__file__).parent.glob("driver_*.cpp")
}

__driver_validator = cv.one_of(*__AVAILABLE_DRIVERS, lower=True, space="_")

__registered_drivers = set()


def driver_validator(value):
    value = __driver_validator(value)
    __registered_drivers.add(value)
    return value


CONF_DRIVERS = "drivers"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_DRIVERS, default=set()): cv.All(
            lambda x: __AVAILABLE_DRIVERS if x == "all" else x,
            {driver_validator},
        ),
    }
)


class WMBusComponentManifest(ComponentManifest):
    exclude_drivers: set[str]

    @property
    def resources(self):
        exclude_files = {f"driver_{name}.cpp" for name in self.exclude_drivers}

        return [fr for fr in super().resources if fr.resource not in exclude_files]


async def to_code(config):
    # Quite hacky:
    # We need to remove extra drivers from src dir

    component = get_component("wmbus_common")
    component.__class__ = WMBusComponentManifest
    component.exclude_drivers = __AVAILABLE_DRIVERS - __registered_drivers
