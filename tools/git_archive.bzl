"""Repository rules for downloading archives from GitHub and GitLab"""

load("@bazel_tools//tools/build_defs/repo:utils.bzl", "get_auth", "patch", "update_attrs", "workspace_and_buildfile")

def _update_integrity_attr(ctx, attrs, download_info):
    # We don't need to override the integrity attribute if sha256 is already specified.
    integrity_override = {} if ctx.attr.sha256 else {"integrity": download_info.integrity}
    return update_attrs(ctx.attr, attrs.keys(), integrity_override)

def _http_request(repository_ctx, urls, strip_prefix, attrs):
    """Utility function to download a file from a URL and return its contents.

    Args:
        repository_ctx: The context object for the repository rule.
        urls: A list of URLs to download from.
        strip_prefix: The prefix to strip from the downloaded archive.
        attrs: The attributes to update with the download information.
    """
    auth = get_auth(repository_ctx, urls)
    download_info = repository_ctx.download_and_extract(
        urls,
        sha256 = repository_ctx.attr.sha256,
        stripPrefix = strip_prefix,
        auth = auth,
    )
    workspace_and_buildfile(repository_ctx)
    patch(repository_ctx, auth = auth)

    return _update_integrity_attr(repository_ctx, attrs, download_info)

def _github_archive_impl(repository_ctx):
    """A rule to be called in the WORKSPACE that adds an external from github using a workspace rule.

    The required name= is the rule name and so is used for @name//... labels when referring to this archive from BUILD files.

    The required commit= is the git hash to download.
    When the git project is also a git submodule in CMake, this should be kept in sync with the git submodule commit used there.
    This can also be a tag.

    The required sha256= is the checksum of the downloaded archive.
    When unsure, you can omit this argument (or comment it out) and then the checksum-mismatch error message message will offer a suggestion.

    The optional build_file= is the BUILD file label to use for building this external.
    When omitted, the BUILD file(s) within the archive will be used.

    The optional local_repository_override= can be used for temporary local testing;
    instead of retrieving the code from github, the code is retrieved from the local filesystem path given in the argument.

    Args:
        repository_ctx: The context object for the repository rule.
    """
    if repository_ctx.attr.build_file and repository_ctx.attr.build_file_content:
        fail("Only one of build_file and build_file_content can be provided.")
    if repository_ctx.attr.workspace_file and repository_ctx.attr.workspace_file_content:
        fail("Only one of workspace_file and workspace_file_content can be provided.")

    repository = repository_ctx.attr.repository
    commit = repository_ctx.attr.commit

    urls = ["https://github.com/%s/archive/%s.tar.gz" % (repository, commit)]

    repository_split = repository.split("/")
    if len(repository_split) != 2:
        fail("The repository must be formatted as 'organization/project'. Got: %s" % repository)
    _, project = repository_split

    # Github archives omit the "v" in version tags, for some reason.
    strip_commit = commit.removeprefix("v")
    strip_prefix = project + "-" + strip_commit

    return _http_request(repository_ctx, urls, strip_prefix, _github_archive_attrs)

def _gitlab_archive_impl(repository_ctx):
    """A macro to be called in the WORKSPACE that adds an external from gitlab using a workspace rule.

    The required name= is the rule name and so is used for @name//... labels when referring to this archive from BUILD files.

    The required commit= is the git hash to download.
    When the git project is also a git submodule in CMake, this should be kept in sync with the git submodule commit used there.
    This can also be a tag.

    The required sha256= is the checksum of the downloaded archive.
    When unsure, you can omit this argument (or comment it out) and then the checksum-mismatch error message message will offer a suggestion.

    The optional build_file= is the BUILD file label to use for building this external.
    When omitted, the BUILD file(s) within the archive will be used.

    The optional local_repository_override= can be used for temporary local testing;
    instead of retrieving the code from github, the code is retrieved from the local filesystem path given in the argument.

    Args:
        repository_ctx: The context object for the repository rule.
    """
    if repository_ctx.attr.build_file and repository_ctx.attr.build_file_content:
        fail("Only one of build_file and build_file_content can be provided.")
    if repository_ctx.attr.workspace_file and repository_ctx.attr.workspace_file_content:
        fail("Only one of workspace_file and workspace_file_content can be provided.")

    repository = repository_ctx.attr.repository
    commit = repository_ctx.attr.commit
    domain = repository_ctx.attr.domain

    repository_split = repository.split("/")
    if len(repository_split) != 2:
        fail("The repository must be formatted as 'organization/project'. Got: %s" % repository)
    _, project = repository_split
    folder_name = project + "-" + commit

    urls = ["https://gitlab.%s/%s/-/archive/%s/%s.tar.gz" % (domain, repository, commit, folder_name)]

    return _http_request(repository_ctx, urls, folder_name, _gitlab_archive_attrs)

_github_archive_attrs = {
    "repository": attr.string(mandatory = True, doc = "The github repository to download from."),
    "commit": attr.string(mandatory = True, doc = "The git commit hash to download."),
    "sha256": attr.string(default = "0" * 64, doc = "The sha256 checksum of the downloaded archive."),
    "build_file": attr.label(doc = "The BUILD file label to use for building this external."),
    "build_file_content": attr.string(doc = "The content for the BUILD file for this repository."),
    "workspace_file": attr.label(doc = "The file to use as the `WORKSPACE` file for this repository."),
    "workspace_file_content": attr.string(doc = "The content for the WORKSPACE file for this repository."),
}

_gitlab_archive_attrs = {
    "repository": attr.string(mandatory = True, doc = "The github repository to download from."),
    "commit": attr.string(mandatory = True, doc = "The git commit hash to download."),
    "sha256": attr.string(default = "0" * 64, doc = "The sha256 checksum of the downloaded archive."),
    "build_file": attr.label(doc = "The BUILD file label to use for building this external."),
    "build_file_content": attr.string(doc = "The content for the BUILD file for this repository."),
    "workspace_file": attr.label(doc = "The file to use as the `WORKSPACE` file for this repository."),
    "workspace_file_content": attr.string(doc = "The content for the WORKSPACE file for this repository."),
    "domain": attr.string(default = "com", doc = "The domain of the gitlab server."),
}

github_archive = repository_rule(
    implementation = _github_archive_impl,
    local = True,
    attrs = _github_archive_attrs,
)

gitlab_archive = repository_rule(
    implementation = _gitlab_archive_impl,
    local = True,
    attrs = _gitlab_archive_attrs,
)
