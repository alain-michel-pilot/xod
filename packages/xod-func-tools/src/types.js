import R from 'ramda';
import $ from 'sanctuary-def';
import $type from 'sanctuary-type-identifiers';
import HMDef from 'hm-def';

/* Types are by convention starts with a capital leter, so: */
/* eslint-disable new-cap */

//-----------------------------------------------------------------------------
//
// Type utilities
//
//-----------------------------------------------------------------------------

// For some reason `type` function from 'sanctuary-type-identifiers' don't work
// correctly with moands from 'ramda-fantasy'. So there is a little hack.
// Also it used to test 'sanctuary-def/Type'.
const checkTypeId = R.curry((expectedType, obj) => {
  const eq = fn => R.compose(R.equals(expectedType), fn);
  return R.anyPass([
    eq(R.path(['constructor', 'prototype', '@@type'])),
    eq($type),
  ])(obj);
});

const $Any = $.NullaryType(
  'xod-func-tools/Any',
  '',
  R.T
);

const $Type = $.NullaryType(
  'sanctuary-def/Type',
  'https://github.com/sanctuary-js/sanctuary-def',
  checkTypeId('sanctuary-def/Type')
);

const def = HMDef.create({
  checkTypes: true,
  env: $.env.concat([
    $Type,
    $Any,
  ]),
});

const removeLastSlash = R.replace(/\/$/, '');
const qualifiedTypeName = def(
  'qualifiedTypeName :: String -> String -> String',
  (packageName, typeName) => `${removeLastSlash(packageName)}/${typeName}`
);
const typeUrl = def(
  'typeUrl :: String -> String -> String',
  (docUrl, typeName) => `${docUrl}${typeName}`
);

export const hasType = def(
  'hasType :: Type -> (x -> Boolean)',
  type => x => type.validate(x).isRight
);

export const hasOneOfType = def(
  'hasOneOfType :: [Type] -> (x -> Boolean)',
  types => R.anyPass(
    R.map(hasType, types)
  )
);

export const NullaryType = def(
  'NullaryType :: String -> String -> String -> (Any -> Boolean) -> Type',
  (packageName, docUrl, typeName, predicate) => $.NullaryType(
    qualifiedTypeName(packageName, typeName),
    typeUrl(docUrl, typeName),
    predicate
  )
);

export const EnumType = def(
  'EnumType :: String -> String -> String -> [a] -> Type',
  (packageName, docUrl, typeName, values) => $.EnumType(
    qualifiedTypeName(packageName, typeName),
    typeUrl(docUrl, typeName),
    values
  )
);

export const Model = def(
  // TODO: Replace `a` with `StrMap Type` (there is error)
  //       after fixing `hm-def`.
  'Model :: String -> String -> String -> a -> Type',
  (packageName, docUrl, typeName, schema) => NullaryType(
    packageName,
    docUrl,
    typeName,
    hasType($.RecordType(schema))
  )
);

export const OneOfType = def(
  'OneOfType :: String -> String -> String -> [Type] -> Type',
  (packageName, docUrl, typeName, types) => NullaryType(
    packageName,
    docUrl,
    typeName,
    hasOneOfType(types)
  )
);

export const AliasType = def(
  'AliasType :: String -> String -> String -> Type -> Type',
  (packageName, docUrl, typeName, type) => NullaryType(
    packageName,
    docUrl,
    typeName,
    hasType(type)
  )
);

//-----------------------------------------------------------------------------
//
// Fantasy land types
//
//-----------------------------------------------------------------------------

const maybeTypeId = 'ramda-fantasy/Maybe';
export const $Maybe = $.UnaryType(
  maybeTypeId,
  'https://github.com/ramda/ramda-fantasy/blob/master/docs/Maybe.md',
  checkTypeId(maybeTypeId),
  maybe => (maybe.isJust ? [maybe.value] : [])
);

const eitherTypeId = 'ramda-fantasy/Either';
export const $Either = $.BinaryType(
  eitherTypeId,
  'https://github.com/ramda/ramda-fantasy/blob/master/docs/Either.md',
  checkTypeId(eitherTypeId),
  either => (either.isLeft ? [either.value] : []),
  either => (either.isRight ? [either.value] : [])
);

//-----------------------------------------------------------------------------
//
// Environment
//
//-----------------------------------------------------------------------------

export const env = $.env.concat([
  $Either,
  $Maybe,
]);

const defType = HMDef.create({ checkTypes: true, env });

export { defType as def };
