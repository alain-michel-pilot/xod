import os from 'os';
import * as R from 'ramda';
import { resolve } from 'path';
import { exec, spawn } from 'child-process-promise';
import YAML from 'yamljs';

import { configure, addPackageIndexUrl, addPackageIndexUrls } from './config';
import parseTable from './parseTable';
import { patchBoardsWithCpu } from './cpuParser';
import listAvailableBoards from './listAvailableBoards';
import parseProgressLog from './parseProgressLog';

const IS_WIN = os.platform() === 'win32';
const escapeSpacesNonWin = R.unless(() => IS_WIN, R.replace(/\s/g, '\\ '));

/**
 * Initializes object to work with `arduino-cli`
 * @param {String} pathToBin Path to `arduino-cli`
 * @param {Object} config Plain-object representation of `.cli-config.yml`
 */
const ArduinoCli = (pathToBin, config = null) => {
  const { path: configPath, config: cfg } = configure(config);

  const escapedConfigPath = escapeSpacesNonWin(configPath);
  const run = args =>
    exec(`"${pathToBin}" --config-file=${escapedConfigPath} ${args}`).then(
      R.prop('stdout')
    );
  const runWithProgress = async (onProgress, args) => {
    const spawnArgs = R.compose(
      R.concat([`--config-file=${escapedConfigPath}`]),
      R.reject(R.isEmpty),
      R.split(' ')
    )(args);

    const promise = spawn(escapeSpacesNonWin(pathToBin), spawnArgs, {
      stdin: 'inherit',
      shell: true,
    });
    const proc = promise.childProcess;

    proc.stdout.on('data', data => onProgress(data.toString()));
    proc.stderr.on('data', data => onProgress(data.toString()));

    return promise.then(R.prop('stdout'));
  };

  const sketch = name => resolve(cfg.sketchbook_path, name);

  const runAndParseTable = args => run(args).then(parseTable);
  const runAndParseJson = args => run(args).then(JSON.parse);

  const listBoardsWith = (listCmd, boardsGetter) =>
    Promise.all([
      runAndParseTable('core list'),
      runAndParseJson(`board ${listCmd} --format json`),
    ]).then(([cores, boards]) =>
      patchBoardsWithCpu(cfg.arduino_data, cores, boardsGetter(boards))
    );

  return {
    dumpConfig: () => run('config dump').then(YAML.parse),
    listConnectedBoards: () => listBoardsWith('list', R.prop('serialBoards')),
    listInstalledBoards: () => listBoardsWith('listall', R.prop('boards')),
    listAvailableBoards: () => listAvailableBoards(cfg.arduino_data),
    compile: (onProgress, fqbn, sketchName, verbose = false) =>
      runWithProgress(
        onProgress,
        `compile --fqbn ${fqbn} ${verbose ? '--verbose' : ''} ${sketch(
          sketchName
        )}`
      ),
    upload: (onProgress, port, fqbn, sketchName, verbose = false) =>
      runWithProgress(
        onProgress,
        `upload --fqbn ${fqbn} --port ${port} ${
          verbose ? '--verbose' : ''
        } -t ${sketch(sketchName)}`
      ),
    core: {
      download: (onProgress, pkgName) =>
        runWithProgress(
          parseProgressLog(onProgress),
          `core download ${pkgName}`
        ),
      install: (onProgress, pkgName) =>
        runWithProgress(
          parseProgressLog(onProgress),
          `core install ${pkgName}`
        ),
      // We have to call our custon `parseTable`
      // until bug with `--format json` in arduino-cli will be fixed
      // https://github.com/arduino/arduino-cli/issues/39
      list: () => runAndParseTable('core list'),
      search: query => runAndParseTable(`core search ${query}`),
      uninstall: pkgName => run(`core uninstall ${pkgName}`),
      updateIndex: () => run('core update-index'),
      upgrade: () => run('core upgrade'),
    },
    version: () => runAndParseJson('version').then(R.prop('version')),
    createSketch: sketchName =>
      run(`sketch new ${sketchName}`).then(
        R.always(resolve(cfg.sketchbook_path, sketchName, `${sketchName}.ino`))
      ),
    addPackageIndexUrl: url => addPackageIndexUrl(configPath, url),
    addPackageIndexUrls: urls => addPackageIndexUrls(configPath, urls),
  };
};

export default ArduinoCli;