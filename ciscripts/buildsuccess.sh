#/bin/bash
curl --silent -XPOST -H "Authorization: token $GITHUB_SECRET" https://api.github.com/repos/wortheumio/wortheum/statuses/$(git rev-parse HEAD) -d "{
  \"state\": \"success\",
  \"target_url\": \"${BUILD_URL}\",
  \"description\": \"Jenkins-CI reports build succeeded!!\",
  \"context\": \"jenkins-ci-wortheum\"
}"
rm -rf $WORKSPACE/*
# make docker cleanup after itself and delete all exited containers
sudo docker rm -v $(docker ps -a -q -f status=exited) || true
