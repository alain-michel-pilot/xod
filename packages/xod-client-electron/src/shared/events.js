// Event to notify Renderer process about some Error in the Main Process
export const ERROR_IN_MAIN_PROCESS = 'ERROR_IN_MAIN_PROCESS';

export const REQUEST_SHOW_PROJECT = 'EVENT_REQUEST_SHOW_PROJECT';
export const REQUEST_CREATE_WORKSPACE = 'EVENT_REQUEST_CREATE_WORKSPACE';

export const CREATE_PROJECT = 'EVENT_CREATE_PROJECT';
export const PROJECT_PATH_CHANGED = 'EVENT_PROJECT_PATH_CHANGED';
export const OPEN_PROJECT = 'EVENT_OPEN_PROJECT';
export const SELECT_PROJECT = 'EVENT_SELECT_PROJECT';
export const OPEN_BUNDLED_PROJECT = 'EVENT_OPEN_BUNDLED_PROJECT';
export const SAVE_ALL = 'EVENT_SAVE_ALL';
export const LOAD_PROJECT = 'EVENT_LOAD_PROJECT';

export const REQUEST_OPEN_PROJECT = 'EVENT_REQUEST_OPEN_PROJECT';
export const CONFIRM_OPEN_PROJECT = 'EVENT_CONFIRM_OPEN_PROJECT';

export const LIST_BOARDS = 'EVENT_LIST_BOARDS';
export const LIST_PORTS = 'EVENT_LIST_PORTS';
export const GET_SELECTED_BOARD = 'EVENT_GET_SELECTED_BOARD';
export const SET_SELECTED_BOARD = 'EVENT_SET_SELECTED_BOARD';

export const SWITCH_WORKSPACE = 'EVENT_SWITCH_WORKSPACE';
export const CREATE_WORKSPACE = 'EVENT_CREATE_WORKSPACE';
export const UPDATE_WORKSPACE = 'UPDATE_WORKSPACE';

export const WORKSPACE_ERROR = 'EVENT_WORKSPACE_ERROR';

export const APP_UPDATE_AVAILABLE = 'EVENT_APP_UPDATE_AVAILABLE';
export const APP_UPDATE_PROGRESS = 'EVENT_APP_UPDATE_PROGRESS';
export const APP_UPDATE_DOWNLOADED = 'EVENT_APP_UPDATE_DOWNLOADED';
export const APP_UPDATE_ERROR = 'EVENT_APP_UPDATE_ERROR';
export const APP_UPDATE_DOWNLOAD_REQUEST = 'EVENT_APP_UPDATE_DOWNLOAD_REQUEST';
export const APP_UPDATE_DOWNLOAD_STARTED = 'EVENT_APP_UPDATE_DOWNLOAD_STARTED';

export const START_DEBUG_SESSION = 'START_DEBUG_SESSION';
export const DEBUG_SESSION = 'DEBUG_SESSION';
export const STOP_DEBUG_SESSION = 'STOP_DEBUG_SESSION';

export const REQUEST_CLOSE_WINDOW = 'REQUEST_CLOSE_WINDOW';
export const CONFIRM_CLOSE_WINDOW = 'CONFIRM_CLOSE_WINDOW';

export const INSTALL_LIBRARIES = 'INSTALL_LIBRARIES';
export const INSTALL_LIBRARIES_COMPLETE = 'INSTALL_LIBRARIES_COMPLETE';
export const INSTALL_LIBRARIES_FAILED = 'INSTALL_LIBRARIES_FAILED';

export const XOD_URL_CLICKED = 'XOD_URL_CLICKED';

export const PAN_TO_CENTER = 'PAN_TO_CENTER';

export const CHECK_ARDUINO_DEPENDENCIES_INSTALLED =
  'CHECK_ARDUINO_DEPENDENCIES_INSTALLED';
export const INSTALL_ARDUINO_DEPENDENCIES = 'INSTALL_ARDUINO_DEPENDENCIES';

export const UPLOAD_TO_ARDUINO = 'UPLOAD_TO_ARDUINO';
