import * as R from 'ramda';
import AtNet from 'at-internet';
import client from 'xod-client';
import { allPromises } from 'xod-func-tools';
import { ipcRenderer } from 'electron';

import { DEBUG_SERIAL_SEND } from '../shared/events';

const DELAY_BETWEEN_CHUNKS = 500;

const formatTetherinErrorMessage = err => ({
  title: 'Tethering Internet Error',
  note: err.message,
});

// Mutable list of chunks to send
// New chunks are placed in the end of list
//
// Transmit function (created by `createTransmitter`) is shifting
// chunks and transmit it using `sender` function, which are different
// for Simulation and Debug mode.
//
let chunksToSend = [];

// Places the new chunks in the end of mutable `chunksToSend` list (SIDE-EFFECT)
// :: [String] -> [String]
const queueChunks = newChunks => {
  chunksToSend = R.concat(chunksToSend, newChunks);
  return chunksToSend;
};

// :: (String -> _) -> (_ -> _)
const createTransmitter = sender => {
  let inProcess = false;

  const transmit = (force = false) => {
    if (chunksToSend.length === 0) {
      inProcess = false;
      return;
    }
    if (inProcess && !force) return;

    inProcess = true;
    const nextChunk = chunksToSend.shift();
    setTimeout(() => {
      // console.log('<', nextChunk);
      sender(nextChunk);
      transmit(chunksToSend, true);
    }, DELAY_BETWEEN_CHUNKS);
  };

  return transmit;
};

// :: String -> [String]
// Pack data into chunks, if needed:
// - Simple AT answers just placed inside array
// - IPD (Incoming Package Data) is splited on chunks of max 64 bytes length
//   with prepended prefix `+XOD:{nodeId}:{dataLength}:`
const splitDataOnChunks = R.curry((nodeId, data) => {
  if (data.length > 0) {
    const prefix = `+XOD:${nodeId}:`;
    const maxChunkSize = 63; // -1 = '\0'
    const pkgSize = 3; // max 2 digits + delimeter: "63:"
    const dataMaxLength = maxChunkSize - pkgSize - prefix.length;

    const formatChunks = chunkData =>
      R.compose(
        R.concat(R.__, '\0'),
        R.concat(prefix),
        R.concat(chunkData.length.toString(10)),
        R.concat(':')
      )(chunkData);

    return R.ifElse(
      R.startsWith('IPD'),
      R.compose(
        R.map(formatChunks),
        R.splitEvery(dataMaxLength),
        R.slice(R.__, -1, data),
        R.add(1),
        R.indexOf(':')
      ),
      R.of
    )(data);
  }
  return [];
});

export default ({ getState, dispatch }) => next => action => {
  const state = getState();
  const result = next(action);

  // Simulation
  if (
    action.type === client.SIMULATION_LAUNCHED &&
    action.payload.tetheringInetNodeId !== null
  ) {
    const nodeId = action.payload.tetheringInetNodeId;
    const worker = action.payload.worker;
    const transmit = createTransmitter(worker.sendToWasm);
    const listener = R.compose(
      transmit,
      queueChunks,
      splitDataOnChunks(nodeId)
    );
    const write = AtNet.create(listener);
    dispatch(
      client.tetheringInetCreated(action.payload.tetheringInetNodeId, write)
    );
  }

  // Debug
  if (
    action.type === client.DEBUG_SESSION_STARTED &&
    action.payload.tetheringInetNodeId !== null
  ) {
    const nodeId = action.payload.tetheringInetNodeId;
    const transmit = createTransmitter(chunk =>
      ipcRenderer.send(DEBUG_SERIAL_SEND, chunk)
    );
    const listener = R.compose(
      transmit,
      queueChunks,
      splitDataOnChunks(nodeId)
    );
    const write = AtNet.create(listener);
    dispatch(
      client.tetheringInetCreated(action.payload.tetheringInetNodeId, write)
    );
  }

  // Send command to AtInternet either for Simulation or Debug
  if (action.type === client.DEBUGGER_LOG_ADD_MESSAGES) {
    const write = client.tetheringInetSender(state);
    if (!write) return result;

    if (R.any(client.isXodNetMessage, action.payload)) {
      R.compose(
        allPromises,
        R.map(R.compose(write, R.prop('content'))),
        R.filter(client.isXodNetMessage)
      )(action.payload).catch(err => {
        dispatch(client.addError(formatTetherinErrorMessage(err)));
        // eslint-disable-next-line no-console
        console.error(err);
      });
    }
  }

  return result;
};
