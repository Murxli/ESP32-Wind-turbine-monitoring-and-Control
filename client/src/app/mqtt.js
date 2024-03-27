// var awsIot = require('aws-iot-device-sdk');
import { fromCognitoIdentityPool } from "@aws-sdk/credential-providers";
import { CognitoIdentityCredentials } from "@aws-sdk/credential-provider-cognito-identity/dist-types/fromCognitoIdentity"

// const device = awsIot.device({
//     clientId: 'ESPMQTTClient',
//     host: 'a3hju2tg3xo1gn-ats.iot.ap-south-1.amazonaws.com',
//     port: 8883,
//     keyPath: './final-private.pem.key',
//     certPath: './final-device.pem.crt',
//     caPath: './final-ca.pem',
// });


// device
//   .on('connect', function() {
//     console.log('connect');
//     device.subscribe('topic_1');
//     device.publish('topic_2', JSON.stringify({ test_data: 1}));
//   });

// device
//   .on('message', function(topic, payload) {
//     console.log('message', topic, payload.toString());
//   });

// device.on("error",(error)=>{
//   console.log(error);
// })

// class AWSCognitoCredentialsProvider extends auth.CredentialsProvider{
//     // private options: AWSCognitoCredentialOptions;
//     // private cachedCredentials? : CognitoIdentityCredentials;

//     constructor(options, expire_interval_in_ms)
//     {
//         super();
//         this.options = options;

//         setInterval(async ()=>{
//             await this.refreshCredentials();
//         },expire_interval_in_ms?? 3600*1000);
//     }

//     getCredentials() {
//         return {
//             aws_access_id: this.cachedCredentials?.accessKeyId ?? "",
//             aws_secret_key: this.cachedCredentials?.secretAccessKey ?? "",
//             aws_sts_token: this.cachedCredentials?.sessionToken,
//             aws_region: this.options.Region
//         }
//     }

//     async refreshCredentials()  {
//         log('Fetching Cognito credentials');
//         this.cachedCredentials = await fromCognitoIdentityPool({
//             // Required. The unique identifier for the identity pool from which an identity should be
//             // retrieved or generated.
//             identityPoolId: this.options.IdentityPoolId,
//             clientConfig: { region: this.options.Region },
//         })();
//     }
// }

