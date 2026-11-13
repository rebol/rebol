REBOL [
    version: 0.2.0
    title: "Rebol/GitHub utilities"
    name: github
    type: module
    date: 8-Nov-2024
    needs: json
    home: https://github.com/Oldes/Rebol-GitHub
    exports: [
        github-query
        github-repo
        github-run
        github-get
        github-post
        github-edit
    ]
]
comment "## Include: %github-rest.reb"
comment {## Title:   "GitHub REST API"}
rest: context [
    api.github: https://api.github.com/
    owner:
    repository: none
    data: make map! 4
    response: none
    use-repo: func [o r] [owner: o repository: r]
    get: object [
        issues: func [] [
            *do 'GET [%repos/ owner %/ repository %/issues] none
        ]
        issue: func [number [integer!]] [
            *do 'GET [%repos/ owner %/ repository %/issues/ number] none
        ]
        issue-comments: func [
            "Gets all comments of an issue by its number"
            number [integer!]
        ] [
            *do 'GET [%repos/ owner %/ repository %/issues/ number %/comments] none
        ]
        issue-labels: func [
            "Gets all labels of an issue by its number"
            number [integer!]
        ] [
            *do 'GET [%repos/ owner %/ repository %/issues/ number %/labels] none
        ]
        current-user: does [*do 'GET %user none]
        workflows: func [] [
            *do 'GET [%repos/ owner %/ repository %/actions/workflows] none
        ]
    ]
    post: object [
        issue: func [
            data [map!] "title, body, labels etc.."
        ] [
            unless block? data/labels [data/labels: reduce [labels]]
            *do 'POST [%repos/ owner %/ repository %/issues] data
        ]
        issue-comment: func [
            "Adds a comment to an issue by its number"
            number [integer!]
            body [string!]
        ] [
            clear data
            data/body: body
            *do 'POST [%repos/ owner %/ repository %/issues/ number %/comments] data
        ]
        issue-label: func [
            "Adds a label(s) to an issue by its number"
            number [integer!]
            label [string! block!]
        ] [
            clear data
            append data/labels: clear [] label
            *do 'POST [%repos/ owner %/ repository %/issues/ number %/labels] data
        ]
        label: func [
            "Creates a label"
            name [string!]
            desc [string!]
            color [string!]
        ] [
            clear data
            data/name: name
            data/description: desc
            data/color: color
            probe data
            *do 'POST [%repos/ owner %/ repository %/labels] data
        ]
        release: func [
            tag_name [string!] "Required. The name of the tag."
            target [string!] {Specifies the commitish value that determines where the Git tag is created from. Can be any branch or commit SHA. Unused if the Git tag already exists. Default: the repository's default branch (usually master).}
            name [string!] "The name of the release."
            body [string! none!] "Text describing the contents of the tag."
            draft [logic!] {true to create a draft (unpublished) release, false to create a published one.}
            prerelease [logic!] {true to identify the release as a prerelease. false to identify the release as a full release.}
        ] [
            clear data
            append data compose [
                tag_name: (tag_name)
                target_commitish: (target)
                name: (name)
                draft: (draft)
                prerelease: (prerelease)
            ]
            ? data
            *do 'POST [%repos/ owner %/ repository %/releases] data
        ]
    ]
    edit: object [
        issue: func [number [integer!] data [map!]] [
            *do 'PATCH [%repos/ owner %/ repository %/issues/ number] data
        ]
    ]
    run: object [
        workflow: func [id] [
            clear data
            data/ref: "master"
            data/inputs: make map! 4
            *do 'POST [%repos/ owner %/ repository %/actions/workflows/ id %/dispatches] data
        ]
    ]
    *do: func [method [word!] path data [map! none!] /local url header] [
        url: join api.github path
        header: make map! 4
        unless header/Authorization: user's github-token [
            do make error! "Authorization token (github-token) is missing!"
        ]
        header/X-OAuth-Scopes: "repo"
        header/Accept: "Accept: application/vnd.github.v3+json"
        if map? data [header/Content-Type: "application/json"]
        response: write url reduce [method to block! header to-json data]
        try [response: load-json to string! response]
    ]
]
comment "-- End of:  %github-rest.reb"
comment "## Include: %github-graphql.reb"
comment {## Title:   "GitHub GraphQL API"}
graphql: context [
    queries: object [
        comment "## Include: %queries.reb"
        comment {## Title:   "Some useful GitHub GraphQL queries"}
        last-20-closed-issues: {
  query($owner:String!,$repo:String!) {
    repository(owner:$owner, name:$repo) {
      issues(last:20, states:CLOSED) {
        edges {
          node {
            title
            url
            labels(first:5) {
              edges {
                node {
                  name
} } } } } } } } }
        repo-labels: {
  query($owner:String!,$repo:String!) {
    repository(owner:$owner, name:$repo) {
      labels(first:100){
        edges {
          node {
            name
            description
            color
} } } } } }
        repo-disk-usage: {
  query ($owner: String!, $repo: String!) {
    repository(owner: $owner, name: $repo) {
    diskUsage
} } }
        list-issues: {
    totalCount
    pageInfo {
      endCursor
    }
    edges {
      node {
        title
        url
        body
        author {login}
        closed
        timelineItems(first: 100) {
          nodes {
            __typename
            ... on IssueComment {
              createdAt
              author {
                login
              }
              body
            }
            ... on CrossReferencedEvent {
              createdAt
              actor {
                login
              }
              url
              source {
                __typename
                ... on Issue {
                  title
                  url
                  id
                }
                ... on PullRequest {
                  title
                  body
                  permalink
                }
              }
              target {
                __typename
                ... on Issue {
                  title
                  url
                  id
                }
                ... on PullRequest {
                  title
                  body
                  permalink
                }
              }
            }
            ... on ClosedEvent {
              createdAt
              actor {
                login
              }
              url
              closer {
                __typename
                ... on PullRequest {
                  number
                  title
                }
                ... on Commit {
                  committedDate
                  id
                  committer {
                    name
                  }
                  commitUrl
                  messageHeadline
                  messageBody
                }
              }
            }
            ... on RenamedTitleEvent {
              createdAt
              actor {login}
              currentTitle
              previousTitle
            }
            ... on MarkedAsDuplicateEvent {
              createdAt
              actor {login}
            }
            ... on LabeledEvent {
              createdAt
              actor {login}
              label {name}
            }
            ... on UnlabeledEvent {
              createdAt
              actor{login}
              label {name}
            }
            ... on SubscribedEvent {
              createdAt
              actor {login}
            }
            ... on UnsubscribedEvent {
              createdAt
              actor {login}
            }
            ... on CommentDeletedEvent {
              createdAt
              actor {login}
            }
            ... on LockedEvent {
              createdAt
              actor {login}
              lockReason
            }
            ... on UnlockedEvent {
              createdAt
              actor {login}
            }
            ... on ReopenedEvent {
              createdAt
              actor {
                login
              }
            }
            ... on ReferencedEvent {
              createdAt
              actor {
                login
              }
              commit {
                committedDate
                id
                committer {
                  name
                }
                commitUrl
                messageHeadline
                messageBody
              }
              commitRepository {
                id
              }
              isCrossRepository
              isDirectReference
            }
            ... on MentionedEvent {
              createdAt
              actor {
                login
              }
              id
            }
          }
        }
        labels(first: 5) {
          edges {
            node {
              name
            }
          }
        }
      }
    }
  }
        first-100-issues: replace {
  query($owner:String!,$repo:String!) {
    repository(owner:$owner, name:$repo) {
      issues(first:100) {
      #ISSUES#
} } } } "#ISSUES#" :list-issues
        next-100-issues: replace {
  query($owner:String!,$repo:String!,$after_issue:String!) {
    repository(owner:$owner, name:$repo) {
      issues(first:100, after:$after_issue) {
      #ISSUES#
} } } } "#ISSUES#" :list-issues
        last-10-commits: {
  query($owner:String!,$repo:String!) {
    repository(owner:$owner, name:$repo) {
  ... on Repository {
      defaultBranchRef {
        target {
          ... on Commit {
            history(first: 10) {
              pageInfo {
                endCursor
                hasNextPage
              }
              edges {
                node {
                  ... on Commit {
                    oid
                    committedDate
                    messageHeadline
                    messageBody
} } } } } } } } } } }
        next-100-commits: {
  query($owner:String!,$repo:String!, $after:String!) {
    repository(owner:$owner, name:$repo) {
  ... on Repository {
      defaultBranchRef {
        target {
          ... on Commit {
            history(first: 100, after: $after) {
              pageInfo {
                endCursor
                hasNextPage
              }
              edges {
                node {
                  ... on Commit {
                    oid
                    committedDate
                    messageHeadline
                    messageBody
} } } } } } } } } } }
        repo-labels: {
  query($owner:String!,$repo:String!) {
  repository(owner:$owner, name:$repo) {
    labels(first:100){
    edges {
      node {
      name
      description
      color
} } } } } }
        releases: {
  query ($owner: String!, $repo: String!) {
    repository(owner: $owner, name: $repo) {
      releases(first: 10) {
        edges {
          node {
            id
            name
            url
          }
        }
      }
    }
  }
}
        comment "-- End of:  %queries.reb"
    ]
    header: make map! [
        Accept: "application/vnd.github+json"
        X-GitHub-Api-Version: "2022-11-28"
    ]
    request: func [
        query [string! word!]
        variables [map! none!]
        /local data retry result
    ] [
        if none? header/Authorization: user's github-token [
            do make error! "Authorization token (github-token) is missing!"
        ]
        if word? :query [
            result: select queries :query
            unless result [
                print [as-purple "*** Unknown query:" as-red :query]
                return none
            ]
            query: :result
        ]
        data: make map! 8
        data/query: query
        if variables [data/variables: variables]
        retry: 3
        while [retry > 0] [
            result: try [
                load-json to-string write https://api.github.com/graphql reduce [
                    'POST
                    to block! any [header []]
                    to-json data
                ]
            ]
            either not error? result [return result] [
                print "GraphQl request error:"
                print result
                print ["^/RETRY:" retry: retry - 1]
            ]
        ]
        none
    ]
]
comment "-- End of:  %github-graphql.reb"
github-query: :graphql/request
github-repo: func [
    "Initialize the repository used for REST API calls."
    repo [block! path!]
] [
    rest/use-repo first repo second repo
    ()
]
github-run: :rest/run
github-get: :rest/get
github-post: :rest/post
github-edit: :rest/edit
