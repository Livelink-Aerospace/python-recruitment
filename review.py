# Python >=3.8
import multiprocessing as mp
import pprint
import random
import time
from typing import Any, Dict, Set
from collections.abc import Mapping

import toml  # requires: pip install toml


class Config:
    def __init__(self, fn, overrides):
            rawconfig = Config.load_toml_file(fn)
            for k, v in overrides.items():
                rawconfig[k] = v
            self._config = rawconfig

    @staticmethod
    def load_toml_file(fn) -> Dict[str, Any]:
        with open(fn, "r") as f:
            return toml.load(f)
        return None

    @property
    def config(self) -> Mapping:
        return self._config

def process_config_updates(myconfig):
    myconfig.config["last_run"] = time.time()
    print(f"Processing config updates, last_run: {myconfig.config['last_run']:.0f}")
    return myconfig

def worker(myconfig: Config) -> None:
    """Worker function to do something interesting and update the config."""
    myconfig = process_config_updates(myconfig)
    time.sleep(1)  # Pretend we are doing some work!

tasks: Set[mp.Process] = set()

while True:
    # Load the config from file
    config = Config("config.toml", {"last_run": None})
    print("Starting with config:")
    pprint.pprint(config.config)

    process = mp.Process(target=worker, args=(config,))
    process.start()
    tasks.add(process)

    completed = {task for task in tasks if not task.is_alive()}

    if completed:
        print(f"Completed {len(completed)} tasks")
        pprint.pprint(config.config)
        tasks.difference_update(completed)

    time.sleep(random.randint(0, 5))

print("Ending with config:")
pprint.pprint(config.config)
