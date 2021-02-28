builddir="$1"
srcdir="$2"

# If no git repo try to read from the existing git_version.h, for building from tarballs
version_h_path="${builddir}/git_version.h"
if ! test -d "${srcdir}/.git"; then
  if test -f "${version_h_path}"; then
    while read line; do
      set -- $line
      export $2=$(echo $3 | sed 's/"//g')
    done < "${version_h_path}"
    if test x$BUILD_GIT_VERSION_NUMBER != x -a x$BUILD_GIT_VERSION_STRING != x; then
      exit 0
    else
      echo "invalid git_version.h"
      exit 2
    fi
  else
    echo "git repo not found and no cached git_version.h"
    exit 2
  fi
fi
osx_bundle_sed_path="${builddir}/osx-bundle.sed"

last_svn_revision=6962
last_svn_hash="16cd907fe7482cb54a7374cd28b8501f138116be"

git_revision=$(expr $last_svn_revision + $(git rev-list --count $last_svn_hash..HEAD))
git_version_str=$(git describe --exact-match 2> /dev/null)
installer_version='0.0.0'
resource_version='0, 0, 0'
if test x$git_version_str != x; then
  git_version_str="${git_version_str##v}"
  tagged_release=1
  if [ $(echo $git_version_str | grep '\d\.\d\.\d') ]; then
    installer_version=$git_version_str
    resource_version=$(echo $git_version_str | sed 's/\./, /g')
  fi
else
  git_branch="$(git symbolic-ref HEAD 2> /dev/null)" || git_branch="(unnamed branch)"
  git_branch="${git_branch##refs/heads/}"
  git_hash=$(git rev-parse --short HEAD)

  git_version_str="${git_revision}-${git_branch}-${git_hash}"
  tagged_release=0
fi

build_date="$(date "+%Y-%m-%d %H:%M %Z")"

new_version_h="\
#define BUILD_GIT_VERSION_NUMBER ${git_revision}
#define BUILD_GIT_VERSION_STRING \"${git_version_str}\"
#define TAGGED_RELEASE ${tagged_release}
#define INSTALLER_VERSION \"${installer_version}\"
#define RESOURCE_BASE_VERSION ${resource_version}"

osx_bundle_sed="\
s/@PLIST_VERSION@/${git_version_str}/g
s/@PLIST_BUILD_DATE@/${build_date}/g
/ *@LOCALIZATIONS@/ {
  r languages
  d
}"

# Write it only if it's changed to avoid spurious rebuilds
# This bizzare comparison method is due to that newlines in shell variables are very exciting
case "$(cat ${version_h_path} 2> /dev/null)"
in
  "${new_version_h}");;
  *)
    echo "${new_version_h}" > "${version_h_path}"
    echo "${osx_bundle_sed}" > "${osx_bundle_sed_path}"
    ;;
esac
