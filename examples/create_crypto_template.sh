#!/bin/bash

if [[ "$#" -lt 1 ]] ;
then
  echo "Usage: ${0} <name>"
  exit -1
fi

name=${1}

dir=$(pwd)

if [[ "${dir}" != */examples ]] ;
then
  echo "This script should be called inside the \"examples\" folder"
  exit -1
fi

mkdir ${name}
if [ $? -ne 0 ]; then
    echo "Failed to create directory ${name}"
    exit -1
fi

option="${name}"
classname="${name^}"
includeguard="${name^^}_H_"

header="${name}/${name}.h"
source="${name}/${name}.cpp"

mkdir "${name}/chars"
mkdir "${name}/testvectors"

# header file template
echo '#ifndef '${includeguard}'
#define '${includeguard}'

#include "crypto.h"

/*! \class '${classname}'
 *  \brief Implementation of '${classname}'
 */
class '${classname}' : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  '${classname}'(cxxopts::Options& options);
};

#endif  // '${includeguard} > "${header}"

# source file template
echo '#include "'${header}'"

#include "bitslice_step.h"
#include "functions.h"
//#include "linear_step.h"

void '${classname}'::AddToOptions(cxxopts::Options& options) {}

'${classname}'::'${classname}'(cxxopts::Options& options) : Crypto(options["word-size"].as<int>()) {
  // TODO: implement '${classname}'
}' > "${source}"

# sources.cmake file template
echo 'set(CRYPTO_FILES
  examples/'${header}'
  examples/'${source}'
)
set(MAP_OPTION_TO_CLASSNAME
  '${option}':'${classname}'
)' > "${name}/sources.cmake"

# xml file template
echo '<config>
  <options>
    <option name="f"  value="'${option}'"/>
    <option name="n"  value="TODO:NUM_ROUNDS"/>
    <option name="w"  value="TODO:WORD_SIZE"/>
  </options>
  <char value=""/>
</config>' > "${name}/chars/${name}_empty.xml"

echo "Successfully created folder and file structure for ${name}"
