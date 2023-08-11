![](https://raw.githubusercontent.com/TexasInstruments/texasinstruments.github.io/main/documents/TI_document_header.png)

**Version:** 1.0

## Table of Contents

[Overview](#overview)
- [Audience](#audience)
- [Links to Important Resources](#links-to-important-resources)

[Github Issues](#github-issues)
- [Reporting Code Bugs](#reporting-code-bugs)
- [Reporting Documentation Bugs](#reporting-documentation-bugs)
- [Proposing Enhancements](#proposing-enhancements)
- [Asking Questions](#asking-questions)

[Submitting a Contribution](#submitting-a-contribution)

- [Git COMMIT Messages](#git-commit-messages)
- [Pull Requests](#pull-requests)
- [Coding Guidelines](#coding-guidelines)
- [Documentation Style Guide](#documentation-style-guide)
- [Testing](#testing)


[First bugs for Contributors](#first-bugs-for-contributors)

# Overview

**Thank you** for taking the time to contribute!  The following is a set of guidelines for how to do so.  Please note that all contributions, whether code, documents, issues, discussion, etc., are subject to this project's [CODE_OF_CONDUCT](https://github.com/TexasInstruments/texasinstruments.github.io/blob/main/documents/CODE_OF_CONDUCT.MD), this [CONTRIBUTING](CONTRIBUTING.MD) document, the Projects [LICENSE](LICENSE.MD) and the Project's [Contributor Agreement](Github_Contributor_Agreement_TIOVX.pdf). Texas Instruments' maintainers for this project will not accept any contribution that is not in accordance with those documents and may also choose to not accept a contribution based on their judgment and evaluation of the change; e.g. license submitted under is not appropriate for the project, does not meet the technical needs of the project, etc.

## Audience

TIOVX is delivered as a component from a larger SDK delivered by Texas Instruments.  It has dependencies on a few other repositories and compilers which are downloaded together as part of device-specific TI Processor SDK's.

Therefore, contributors are expected to download the relevant Processor SDK for the TI device they are working on as part of building and developing with TIOVX.  A recommended workflow would be to start with a specific version of the Processor SDK, and replace the TIOVX folder in the download with a clone of this repository (other repos in the SDK may also be available to clone as well).

For testing, contributors would ideally have either their own development board with the relevant TI Device, or one of the development boards offered by TI or our partners.

As this project is intended to be used in production safety-critical systems, maintainers are required to assess the value, quality, and risk associated with each proposed submission.  For example, it may be that a feature may be rejected based on the high cost of the ongoing maintenance associated with it when compared to the probability of it being used in multiple production systems.  Users are always free to maintain their own forks.

## Links to Important Resources

* **Docs:**
  * [TIOVX User Guide](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/) - TI's TIOVX user guide from latest release
  * [OpenVX 1.1 Specification](https://www.khronos.org/registry/OpenVX/specs/1.1/html/index.html) - Khronos OpenVX 1.1 Specification
  * [Resources](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/RESOURCES.html) - Sub-page from TIOVX user guide which contains several useful links to OpenVX standard specifications, videos, tutorials, presentations, papers, and FAQs.

* **Software/Hardware Questions and Support:**
  * [TI E2E Forum](https://e2e.ti.com/support/processors-group/processors/f/processors-forum) - Forum where you can search and ask questions of the broader TI community of developers.

* **Questions about contributions:**  You can file issues on this github project as per this document.

* **Project Contributor Agreement:**  [CONTRIBUTOR AGREEMENT](Github_Contributor_Agreement_TIOVX.pdf)

* **Project License:**  [LICENSE](LICENSE.MD)

* **Code of Conduct:**   [CODE_OF_CONDUCT](https://github.com/TexasInstruments/texasinstruments.github.io/blob/main/documents/CODE_OF_CONDUCT.MD)

# Github Issues

This section is related to the use of github issues to report bugs in code or documentation, as well as to propose enhancements or ask questions.

## Reporting Code Bugs

Bugs are tracked as [GitHub issues](https://guides.github.com/features/issues/). When submitting the issue:

- Label the Issue with the **bug** Label.

- **Use a clear and descriptive title** for the issue to identify the problem.

- Provide the following information by copying in [BUG_TEMPLATE](BUG_TEMPLATE.MD).

*Please note that there may be cases where a maintainer may need to recategorize a "bug" to be an "enhancement request" if they find that the reported behavior was never captured as a software requirement in the first place.  Since we track these requirements internally, we understand it is not always obvious if a behavior is due to a bug or a missing requirement.*

## Reporting Documentation Bugs

Issues with Documentation are tracked as [GitHub issues](https://guides.github.com/features/issues/).  When submitting the issue:

- Label the Issue with the **documentation**  Label.

- Use a clear and descriptive title that states the problem. E.g. "Build instructions page has misspellings"" OR "Build instructions are not complete".

- Describe the issue with the documentation in as much detail as possible.

- Include screen shots of or links to the documentation if relevant.

## Proposing Enhancements

Enhancements are features that you wish to propose, but aren't necessarily bugs/problems with the existing code or documentation. Enhancements should be discussed using a [GitHub issue](https://guides.github.com/features/issues/) and the label should be  **enhancement**.

When submitting a proposal for an Enhancement or New Feature, please attach a Markdown document that describes what the feature is and the proposed implementation.

The Enhancement will be reviewed by the Project Maintainers and the Community at large and if there is consensus you can move forward with it.

Please note, that this project is the TI maintained implementation of OpenVX.  In general, it is preferred to submit new features to the Khronos OpenVX working group for consideration to be added to the main OpenVX specification or as an official Khronos extension.  This can be done in one of two ways:

- [OpenVX Community Forum](https://community.khronos.org/c/openvx/22) - Open forum to submit feature requests, ask questions, and give feedback to the OpenVX working group and broader community.

- [Khronos Membership](https://www.khronos.org/members/) - You can join Khronos and participate in the OpenVX working group to define new features and improve on the specification.

## Asking Questions

[GitHub issues](https://guides.github.com/features/issues/) can also be used to ask specific questions related to the TI implementation of OpenVX or this github project in general.  For other questions, please use the following:

- [Khronos OpenVX Community Forum](https://community.khronos.org/c/openvx/22) - Khronos hosted forum for OpenVX specification related questions.

- [TI E2E Forum](https://e2e.ti.com/support/processors-group/processors/f/processors-forum) - TI hosted forum where you can search and ask questions related to TI devices and software.


# Submitting a Contribution

Whether contributions are for fixing bugs, or adding enhancements, please open a [pull request](). If you are not sure whether your changes are correct or you want to get early discussion going prior to completing, you can either mark it as [draft](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-pull-requests#draft-pull-requests) or create an issue to discuss the problem and possible ways to fix it prior to publishing a PR.

We recommend to use the [Fork and Pull Request workflow](https://docs.github.com/en/get-started/quickstart/contributing-to-projects).

**Acceptance Criteria**: Please ensure that your contribution adheres to the requirements and recommendations listed in this document.  As this project is intended to be used in production safety-critical systems, maintainers are required to assess the value, quality, and risk associated with each proposed submission.  For example, it may be that a feature may be rejected based on the high cost of the ongoing maintenance associated with it when compared to the probability of it being used in multiple production systems.  Users are always free to maintain their own forks.

In time, we hope to deploy auto checkers on each Pull Request to give feedback from automated tests and static analysis checkers (*pending*).

**Responsiveness**: Maintainers of this project have responsibilities beyond maintaining this project.  Therefore the responsiveness evaluating community posted bugs and features may vary.

Each functional change (new feature or bug fix) must be supplied with corresponding tests. See [Testing](#testing) for more information about testing. NFC (non-functional change) PRs can be accepted without new tests.

**bosch is also contributing**

**bosch is also contributing 2**

## Git COMMIT Messages

COMMIT messages are important for both reviewers and future contributors to be able to understand what was done in a commit, and why the change was done.  Therefore, the COMMIT message shall answer the following questions as part of the comments:
1. What high level changes are included in the COMMIT
2. Why were these changes done

The format of the commit message shall be as follows:

```
[Unique ISSUE Identifier] Summary line

- Bulleted list of comments describing if it is a
  bug fix or a feature, what is being changed, and why.
```
The "Unique ISSUE Identifier" for TI internal contributions have used JIRA issues historically.  External contributions can use github issue identifiers (either bug or enhancements) for traceability and ease of linking to the associated issue.

## Pull Requests

When creating a pull request, please adhere to following guidelines:

- Apply the appropriate tag (bug, enhancement, etc)
- Submit to merge to the appropriate branch (main is default)
- If you want to share the PR in draft state, then you can use the [Draft Pull Request](https://github.blog/2019-02-14-introducing-draft-pull-requests/) feature to indicate that it is not ready to merge yet, but want to start the conversation.
- During review, it is okay to keep adding several commits in the PR, but each of them should be buildable and tests should pass.  After finalizing the patch prior to merging, the submitter is requested to rebase and squash several commits into a minimal logical subset (removing intermediate modifications based on review comments).

## Coding Guidelines

The code formatting and rules vary between whether the contribution is part of the [production source](#production-source), or part of the [tests](#tests-scripts).  The following rules are common to both:

* **Whitespace** - indent 4 spaces, no tabs

### Production Source

If the change is in code which is expected to be deployed on a production system (everything but tests, scripts, and standalone documentation), then it must follow the following specific guidelines:

TI internally uses [Klocwork](https://help.klocwork.com/) static analysis checker to report static analysis errors and warnings to resolve.

* [Klocwork C Checker Reference](https://help.klocwork.com/current/en-us/concepts/candccheckerreference.htm) - Standard static analysis checks for C
* [Klocwork MISRA C Checker](https://help.klocwork.com/current/en-us/concepts/misraccheckerreference_2012nolinks.htm) - restricted subset of the C language as defined by [MISRA](https://www.misra.org.uk/)

In general, it is good to follow existing coding convention patterns as established in this project.  If you see an inconsistency or have a question, feel free to submit an issue to get clarity.  This section may grow as the maintainers see common issues in contributions.

### Tests

In general, test code and scripts are not required to follow the static analysis requirements of [Production Source](#production-source).  However, it should still follow the existing coding convention patterns of existing tests. Also, please consider following specific requirements:

* **Included Headers** - Please only include public headers and not internal framework headers (i.e. do not use headers from *source/include* folder).  The idea is to test only the APIs which are available as public interfaces.
* **Buffer Allocations** - For buffer allocations in the tests, be sure to use the *ct_* prefixed allocation functions (like *ct_alloc_mem* or *ct_calloc*) instead of *malloc* or *calloc*.

## Documentation Style Guide

TIOVX makes use of [Doxygen](https://www.doxygen.nl/) for formatting comments and generating documentation.  Contributions shall format comments in conformance with Doxygen style for consistent documentation generation from source code.

## Testing

All contributions must be tested and suitable test cases must be added when required. This section gives details necessary to add or update test cases.

### Test Application

The Khronos OpenVX working group has released an OpenVX standard conformance test suite that any conformant implementation must pass to be considered *conformant*.  TI has included this test suite in the *tiovx/conformance_test* folder of the repo, and we have augmented it with additional tests to fill test gaps, add negative testing, and add tests for TI extensions.  All of the tests are compiled together into a single executable as shown in the table below:

| Type         | Location |
| :----------- | :------- |
| PC Emulation | tiovx/out/PC/x86_64/LINUX/\<profile\>/ vx_conformance_tests_exe |
| Target       | vision_apps/out/\<PLATFORM\>/\<CORE\>/\<HLOS\>/\<PROFILE\>/vx_app_conformance.out |

**Building:**
The test application is built by default as part of the SDK build.  Refer to the [user guide](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/BUILD_INSTRUCTIONS.html#BUILD_EVM) for more details on build, and build options for turning on/off whole segments of the build for tests.

The test application can be run in its entirety:
```
./vx_app_conformance.out
```
or with an optional command line "--filter=" to narrow down to a subset of test vectors:
```
./vx_app_conformance.out --filter=tivxGraph*
```

**Source Location:**
Refer to the [directory structure](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/index.html#TIOVX_PACKAGE_CONTENTS) for details to where the tests are located.  Most of the test framework is in the *conformance_tests* directory, and most of the kernel specific tests are found in the kernels directories in a *test* subfolder.
```
tiovx/conformance_tests
tiovx/kernels/*/test
```
If you want to add tests for framework features, then they should be added to the following folder:
```
tiovx/conformance_tests/test_tiovx
```
For adding tests to the kernels, use the respective kernels test folder.  For example, for kernels added to tiovx repo, the following is the appropriate test folder:
```
kernels/openvx-ext/test
```
**_IMPORTANT:_**
Please avoid modifying contents of the following folder:
```
tiovx/conformance_tests/test_conformance
```

This folder is where the original Khronos OpenVX conformance tests reside, and we try to avoid modifications so that it will be simple to move to a different version of the spec in the future by wholesale updating this folder only.

**Updating or adding a test:**
The test framework is based on [Google Test](http://google.github.io/googletest/), so you can refer to that documentation for a primer.  You can also refer to existing tests and git history for when others have added/updated tests.

**Test Coverage:**
The use of a code coverage tool is recommended to ensure that sufficient positive and negative test coverage is added for the added feature or bug fix.


# First bugs for Contributors

First time contributors should look for the **"good first issue"** label of any outstanding bugs.  These will be provided for issues which it is suspected to be isolated to a relatively small subset of the code (like a specific data object, or callback).
