import esphome.config_validation as cv
from esphome.const import SOURCE_FILE_EXTENSIONS
from esphome.loader import get_component, ComponentManifest
from pathlib import Path

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

AVAILABLE_DRIVERS = {
    f.stem.removeprefix("driver_") for f in Path(__file__).parent.glob("driver_*.cc")
}

driver_validator = cv.one_of(*AVAILABLE_DRIVERS, lower=True, space="_")
registered_drivers = set()


def validate_driver(value):
    value = driver_validator(value)
    registered_drivers.add(value)
    return value


CONF_DRIVERS = "drivers"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_DRIVERS, default=set()): cv.All(
            lambda x: AVAILABLE_DRIVERS if x == "all" else x,
            {validate_driver},
        ),
    }
)


class WMBusComponentManifest(ComponentManifest):
    exclude_drivers: set[str]

    @property
    def resources(self):
        exclude_files = {f"driver_{name}.cc" for name in self.exclude_drivers}
        SOURCE_FILE_EXTENSIONS.add(".cc")
        resources = [fr for fr in super().resources if fr.resource not in exclude_files]
        SOURCE_FILE_EXTENSIONS.discard(".cc")
        return resources


async def to_code(config):
    component = get_component("wmbus_common")
    component.__class__ = WMBusComponentManifest
    component.exclude_drivers = AVAILABLE_DRIVERS - registered_drivers
